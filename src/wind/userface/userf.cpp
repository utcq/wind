#include <wind/userface/userf.h>

#include <wind/processing/lexer.h>
#include <wind/processing/parser.h>
#include <wind/bridge/ast.h>
#include <wind/bridge/ast_printer.h>
#include <wind/generation/compiler.h>
#include <wind/generation/optimizer.h>
#include <wind/generation/ir_printer.h>
#include <wind/generation/backend.h>
#include <wind/generation/ld.h>

#include <filesystem>
#include <iostream>

const char HELP[] = "Usage: wind [options] [files]\n"
                    "Options:\n"
                    "  -ej  Emit object file\n"
                    "  -o   Output file path\n"
                    "  -sa  Show AST\n"
                    "  -si  Show IR\n"
                    "  -h   Display this help message\n";

WindUserInterface::WindUserInterface(int argc, char **argv) {
  this->flags = 0;
  this->argv = argv;
  for (int i = 1; i < argc; i++) {
    parseArgument(std::string(argv[i]), i);
  }
}

void WindUserInterface::parseArgument(std::string arg, int &i) {
  if (arg == "-ej") {
    this->flags |= EMIT_OBJECT;
  }
  else if (arg == "-o") {
    this->output = std::string(argv[++i]);
  }
  else if (arg == "-sa") {
    this->flags |= SHOW_AST;
  }
  else if (arg == "-si") {
    this->flags |= SHOW_IR;
  }
  else if (arg == "-h") {
    std::cout << HELP;
    _Exit(0);
  }
  else {
    files.push_back(arg);
  }
}

void WindUserInterface::emitObject(std::string path) {
  WindLexer *lexer = TokenizeFile(path.c_str());
  if (lexer == nullptr) {
    std::cerr << "File not found: " << path << std::endl;
    _Exit(1);
  }
  WindParser *parser = new WindParser(lexer->get(), lexer->source());
  Body *ast = parser->parse();
  if (flags & SHOW_AST) {
    std::cout << "[" << path << "] AST:" << std::endl;
    ASTPrinter *printer = new ASTPrinter();
    ast->accept(*printer);
    std::cout << "\n\n";
  }

  WindCompiler *ir = new WindCompiler(ast);
  WindOptimizer *opt = new WindOptimizer(ir->get());
  IRBody *optimized = opt->get();

  if (flags & SHOW_IR) {
    std::cout << "[" << path << "] IR:" << std::endl;
    IRPrinter *ir_printer = new IRPrinter(optimized);
    ir_printer->print();
    std::cout << "\n\n";
  }

  WindEmitter *backend = new WindEmitter(optimized);
  backend->process();
  std::string output = "";
  if (this->flags & EMIT_OBJECT && this->files.size()==1 && this->output != "") {
    output = backend->emitObj(this->output);
  } else {
    output = backend->emitObj();
  }
  this->objects.push_back(output);

  delete ir;
  delete opt;
  delete backend;
}

void WindUserInterface::ldDefFlags(WindLdInterface *ld) {
  ld->addFlag("-m elf_x86_64");
}

void WindUserInterface::ldExecFlags(WindLdInterface *ld) {
  ld->addFlag("-dynamic-linker /lib64/ld-linux-x86-64.so.2");
  ld->addFlag("-lc");
  ld->addFile("raw_std/wind_lib.o");
}

void WindUserInterface::processFiles() {
  if (files.size() == 0) {
    std::cerr << "No input file provided\n";
    _Exit(1);
  }
  for (std::string file : files) {
    this->emitObject(file);
  }

  WindLdInterface *ld = new WindLdInterface(this->output);
  ldDefFlags(ld);
  if (this->flags & EMIT_OBJECT) {
    if (this->files.size()==1 && this->output != "") {
      delete ld;
      return;
    }
    ld->addFlag("-r");
  } else {
    ldExecFlags(ld);
  }
  for (std::string obj : this->objects) {
    ld->addFile(obj);
  }
  ld->link();

  delete ld;
}