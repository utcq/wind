#include <wind/generation/IR.h>
#include <wind/generation/backend.h>
#include <wind/common/debug.h>
#include <wind/generation/gas.h>
#include <asmjit/asmjit.h>
#include <fstream>
#include <math.h>
#include <filesystem>
#include <random>
#include <map>
#include <regex>

const asmjit::x86::Gp SystemVABI[] = {
  asmjit::x86::rdi,
  asmjit::x86::rsi,
  asmjit::x86::rdx,
  asmjit::x86::rcx,
  asmjit::x86::r8,
  asmjit::x86::r9
};

void WindEmitter::InitializeSections() {
  this->text = this->code_holder.textSection();
  this->code_holder.newSection(
    &this->data, ".data", 6,
    asmjit::SectionFlags::kNone,
    1,
    1
  );
  this->code_holder.newSection(
    &this->bss, ".bss", 5,
    asmjit::SectionFlags::kZeroInitialized,
    1,
    2
  );
  this->code_holder.newSection(
    &this->rodata, ".rodata", 8,
    asmjit::SectionFlags::kReadOnly,
    1,
    3
  );
}

WindEmitter::WindEmitter(IRBody *program) : program(program) {
  this->code_holder.init(asmjit::Environment::host());
  this->InitializeSections();
  this->assembler = new asmjit::x86::Assembler(&this->code_holder);
  this->logger = new asmjit::StringLogger();
  this->logger->content().appendFormat(".intel_syntax noprefix\n");
  this->logger->setFlags(asmjit::FormatFlags::kHexOffsets);
  this->assembler->setLogger(this->logger);
  this->assembler->section(this->text);
  this->string_table.table = new std::map<std::string, std::string>();
  this->reg_vars = new std::map<int, asmjit::x86::Gp>();
  this->secHeader();
}

WindEmitter::~WindEmitter() {}

int nextpow2(int n) {
  return pow(2, ceil(log2(n)));
}

void WindEmitter::emitPrologue() {
  if (
    (current_function->isStack() && current_function->used_offsets.size() > 0)
    || current_function->flags & PURE_LOGUE
  ) {
    this->assembler->push(asmjit::x86::rbp);
    this->assembler->mov(asmjit::x86::rbp, asmjit::x86::rsp);
    if (current_function->isCallSub()) {
      this->assembler->sub(asmjit::x86::rsp, 16);
    }
  }
  this->canaryPrologue();
}

void WindEmitter::emitEpilogue() {
  this->canaryEpilogue();
  if (
    (current_function->isStack() && current_function->used_offsets.size() > 0) 
    || current_function->flags & PURE_LOGUE
  ) {
    this->assembler->leave();
  }
  this->assembler->ret();
}

void WindEmitter::emitFunction(IRFunction *fn) {
  this->reg_vars->clear();
  if (fn->name() == "main") {
    this->logger->content().appendFormat(".global main\n");
  }
  current_function = fn;
  this->cconv_index = 0;
  this->assembler->bind(this->assembler->newNamedLabel(fn->name().c_str()));
  this->emitPrologue();
  for (auto &statement : fn->body()->get()) {
    this->emitNode(statement.get());
  }
  this->emitEpilogue();
}

asmjit::x86::Gp WindEmitter::adaptReg(asmjit::x86::Gp reg, int size) {
  switch (size) {
    case 1:
      return reg.r8();
    case 2:
      return reg.r16();
    case 4:
      return reg.r32();
    case 8:
      return reg.r64();
    default: {
      throw std::runtime_error("Invalid size");
    }
  } 
}

IRFunction *WindEmitter::FindFunction(std::string name) {
  for (auto &statement : this->program->get()) {
    if (statement->type() == IRNode::NodeType::FUNCTION) {
      IRFunction *fn = statement->as<IRFunction>();
      if (fn->name() == name) {
        return fn;
      }
    }
  }
  return nullptr;
}

void WindEmitter::SolveCArg(IRNode *arg, int type) {
  if (this->cconv_index < 6) {
    this->emitExpr(arg, this->adaptReg(SystemVABI[this->cconv_index], type));
  } else {
    asmjit::x86::Gp reg = this->emitExpr(arg, this->adaptReg(asmjit::x86::rax, type));
    this->assembler->push(reg);
  }
  this->cconv_index++;
}

asmjit::x86::Gp WindEmitter::emitFunctionCall(IRFnCall *fn_call) {
  this->cconv_index = 0;
  IRFunction *fn = this->FindFunction(fn_call->name());
  if (!fn) {
    throw std::runtime_error("Function not found");
  }
  for (size_t i = 0; i < fn_call->args().size(); i++) {
    this->SolveCArg(fn_call->args()[i].get(), fn->GetArgSize(i));
  }
  this->assembler->call(this->assembler->labelByName(fn->name().c_str()));
  return this->adaptReg(asmjit::x86::rax, fn->ret_size);
}

asmjit::x86::Gp WindEmitter::emitExpr(IRNode *node, asmjit::x86::Gp dest) {
  switch (node->type()) {

    case IRNode::NodeType::LITERAL : {
      IRLiteral *lit = node->as<IRLiteral>();
      return this->emitLiteral(lit, dest);
    }

    case IRNode::NodeType::LOCAL_REF : {
      IRLocalRef *local = node->as<IRLocalRef>();
      return this->moveVar(local, dest);
    }

    case IRNode::NodeType::FUNCTION_CALL : {
      IRFnCall *fn_call = node->as<IRFnCall>();
      return this->emitFunctionCall(fn_call);
    }

    case IRNode::NodeType::BIN_OP : {
      IRBinOp *bin_op = node->as<IRBinOp>();
      return this->emitBinOp(bin_op, dest);
    }

    default: {
      std::cout << "Unknown node type" << std::endl;
      break;
    }
  }
  return dest;
}

void WindEmitter::SolveArg(IRArgDecl *decl) {
  IRLocalRef *local = decl->local();
  if (this->cconv_index < 6) {
    this->assembler->mov(
      asmjit::x86::ptr(asmjit::x86::rbp, -local->offset(), local->size()),
      this->adaptReg(SystemVABI[this->cconv_index], local->size())
    );
  } else {
    asmjit::x86::Gp rax = this->adaptReg(asmjit::x86::rax, local->size());
    this->assembler->mov(
      rax, asmjit::x86::ptr(asmjit::x86::rbp, (this->cconv_index-6)*8, local->size())
    );
    this->assembler->mov(
      asmjit::x86::ptr(asmjit::x86::rbp, -local->offset(), local->size()), rax
    );
  }
  this->cconv_index++;
}

void WindEmitter::emitReturn(IRRet *ret) {
  this->emitExpr(ret->get(), asmjit::x86::rax);
  //asmjit::x86::Gp target_reg = this->adaptReg(asmjit::x86::rax, this->current_function->ret_size);
}

void WindEmitter::emitAsm(IRInlineAsm *asm_node) {
  std::string code = asm_node->code();
  code.pop_back();
  std::regex localRefPattern(R"(\?\w+)");
  std::smatch match;
  while (std::regex_search(code, match, localRefPattern)) {
    std::string localRef = match.str().substr(1);
    IRLocalRef *local = this->current_function->GetLocal(localRef);
    std::string ref = "[rbp-" + std::to_string(local->offset()) + "]";
    code = std::regex_replace(code, localRefPattern, ref);
  }
  this->logger->content().appendFormat("%s\n", code.c_str());
}

void WindEmitter::emitNode(IRNode *node) {
  switch (node->type()) {
    case IRNode::NodeType::FUNCTION : {
      this->emitFunction(node->as<IRFunction>());
      break;
    }
    case IRNode::NodeType::RET : {
      IRRet *ret = node->as<IRRet>();
      this->emitReturn(ret);
      break;
    }
    case IRNode::NodeType::LOCAL_DECL : {
      IRLocalDecl *local_decl = node->as<IRLocalDecl>();
      if (local_decl->value()) {
        this->moveIntoVar(local_decl->local(), local_decl->value());
      }
      break;
    }
    case IRNode::NodeType::ARG_DECL : {
      IRArgDecl *arg_decl = node->as<IRArgDecl>();
      this->SolveArg(arg_decl);
      break;
    }
    case IRNode::NodeType::IN_ASM : {
      IRInlineAsm *asm_node = node->as<IRInlineAsm>();
      this->emitAsm(asm_node);
      break;
    }
    default: {
      std::cout << "Unknown node type" << std::endl;
      break;
    }
  }
}

void cleanLoggerContent(std::string& outemit) {
  std::regex ptrPattern(R"(\b\w+word\s+ptr\b\s)");
  outemit = std::regex_replace(outemit, ptrPattern, "");
  std::regex sectionPattern(R"(\.section\s+([^\s]+)\s+\{\#[0-9]+\})");
  outemit = std::regex_replace(outemit, sectionPattern, ".section $1");
}

std::string generateRandomFilePath(const std::string& directory, const std::string& extension) {
    std::string tempDir = directory.empty() ? std::filesystem::temp_directory_path().string() : directory;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(100000, 999999);
    std::string randomFileName = "tmp" + std::to_string(dist(gen)) + extension;
    return tempDir + "/" + randomFileName;
}

std::string WindEmitter::newRoString(std::string str) {
  std::string label = ".RLC" + std::to_string(this->rostring_i);
  this->string_table.table->insert(std::pair<std::string, std::string>(label, str));
  this->rostring_i++;
  return label;
}

std::string WindEmitter::emit() {
  for (auto &statement : this->program->get()) {
    this->emitNode(statement.get());
  }

  this->code_holder.flatten();
  this->code_holder.resolveUnresolvedLinks();

  this->assembler->section(this->rodata);
  for (auto &entry : *this->string_table.table) {
    this->assembler->bind(this->assembler->newNamedLabel(entry.first.c_str()));
    this->logger->content().appendFormat(".string \"%s\"\n", entry.second.c_str());
  }

  std::string outemit = this->logger->content().data();\
  cleanLoggerContent(outemit);
  std::string path = generateRandomFilePath("", ".S");
  std::ofstream file(path);
  if (file.is_open()) {
    file << outemit;
    file.close();
  }
  WindGasInterface *gas = new WindGasInterface(path);
  return gas->assemble();
}