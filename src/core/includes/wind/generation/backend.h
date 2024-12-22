#include <wind/generation/IR.h>
#include <asmjit/asmjit.h>
#include <map>
#include <functional>

#ifndef BACKEND_H
#define BACKEND_H

struct StringTable {
  std::map<std::string, std::string> *table;
};

class WindEmitter {
public:
  WindEmitter(IRBody *program);
  virtual ~WindEmitter();
  void process();
  std::string emitObj(std::string outpath="");
  asmjit::StringLogger *logger;

private:
  IRBody *program;
  IRFunction *current_function;
  int cconv_index = 0; // reset on each function call/decl
  unsigned rostring_i=0;
  unsigned branch_i=0;

  typedef std::function<void(asmjit::Label)> asmjit_jmp_generic;
  std::map<IRBinOp::Operation, asmjit_jmp_generic[2]> jmp_map;

  // Asmjit
  asmjit::x86::Assembler *assembler;
  asmjit::CodeHolder code_holder;
  asmjit::Section *text;
  asmjit::Section *data;
  asmjit::Section *bss;
  asmjit::Section *rodata;

  // Assembly optimization
  // Tracking local vars in registers
  std::map<int, asmjit::x86::Gp> *reg_vars;
  void OptVarMoved(int offset, asmjit::x86::Gp reg);
  asmjit::x86::Gp OptVarGet(int offset);
  void OptClearReg(asmjit::x86::Gp reg);
  void OptTabulaRasa(); // Cool name, right?
  std::string OptCommonRed(std::string asmr);

  // Security
  void secHeader();
  void canaryPrologue();
  void canaryEpilogue();

  // Strings
  StringTable string_table;

  std::string newRoString(std::string str);
  void InitializeSections();
  void emitNode(IRNode *node);
  void emitReturn(IRRet *ret);
  void emitBranch(IRBranching *branch);
  void emitLoop(IRLooping *loop);
  void LitIntoVar(IRLocalRef *local, IRLiteral *lit);
  void moveIntoIndex(const IRLocalAddrRef *local, IRNode *value);
  IRFunction *FindFunction(std::string name);
  asmjit::x86::Gp emitFunctionCall(IRFnCall *fn_call, asmjit::x86::Gp dest);
  void SolveCArg(IRNode *arg, DataType* type);
  void emitFunction(IRFunction *fn);
  void emitPrologue();
  void emitEpilogue();
  void emitAsm(IRInlineAsm *asm_node);
  asmjit::x86::Gp emitExpr(IRNode *node, asmjit::x86::Gp dest, bool isJmp=false);
  void emitCJmp(IRNode *node, asmjit::Label label, bool invert=false);
  asmjit::x86::Gp moveVar(IRLocalRef *local, asmjit::x86::Gp dest);
  asmjit::x86::Gp emitLRef(IRLocalAddrRef *local, asmjit::x86::Gp dest);
  asmjit::x86::Gp adaptReg(asmjit::x86::Gp reg, int size);
  void SolveArg(IRArgDecl *decl);
  void moveIntoVar(IRLocalRef *local, IRNode *value);
  asmjit::x86::Gp emitBinOp(IRBinOp *bin_op, asmjit::x86::Gp dest, bool isJmp=false);
  asmjit::x86::Gp emitLiteral(IRLiteral *lit, asmjit::x86::Gp dest);
  asmjit::x86::Gp emitString(IRStringLiteral *str, asmjit::x86::Gp dest);
  void moveIfNot(asmjit::x86::Gp dest, asmjit::x86::Gp src);
  asmjit::Label newBranchLabel();
};

#endif