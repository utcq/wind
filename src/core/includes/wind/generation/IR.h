#include <algorithm>
#ifndef IR_H
#define IR_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <wind/bridge/opt_flags.h>

class IRNode {
public:
  enum class NodeType {
    RET,
    LOCAL_REF,
    GLOBAL_REF,
    BODY,
    FUNCTION,
    BIN_OP,
    LITERAL,
    STRING,
    LOCAL_DECL,
    GLOBAL_DECL,
    ARG_DECL,
    FUNCTION_CALL,
    IN_ASM,
    LADDR_REF,
    BRANCH,
    LOOP,
    BREAK,
    CONTINUE,
    FN_REF
  };

  virtual ~IRNode() = default;
  virtual NodeType type() const = 0;

  template <typename T>
  bool is() const {
    return dynamic_cast<const T*>(this) != nullptr;
  }

  template <typename T>
  T* as() {
    return dynamic_cast<T*>(this);
  }

  template <typename T>
  const T* as() const {
    return dynamic_cast<const T*>(this);
  }
};

class DataType {
  private:
    DataType *array;
    uint16_t type_size;
    uint16_t capacity;
    bool signed_type = true;

  public:
    DataType(uint16_t size, bool is_signed) : type_size(size), capacity(1), array(nullptr), signed_type(is_signed) {}
    DataType(uint16_t size, uint16_t cap) : type_size(size), capacity(cap), array(nullptr) {}
    DataType(uint16_t size, DataType *arr) : type_size(size), capacity(UINT16_MAX), array(arr) {}
    DataType(uint16_t size, uint16_t cap, DataType *arr) : type_size(size), capacity(cap), array(arr) {}
    bool isArray() const { return array != nullptr; }
    bool isSigned() const { return signed_type; }
    uint16_t moveSize() const { if (!isArray()) return type_size; return 8; }
    uint16_t index2offset(uint16_t index) const {
      if (isArray()) { return index * type_size; }
      return index;
    }
    uint16_t memSize() const {
      uint32_t bytes = 0;
      if (!isArray()) {
        return type_size;
      }
      if (capacity != UINT16_MAX) {
        bytes = array->memSize() * capacity;
      } else {
        bytes = array->memSize() + Sizes::QWORD;
      }
      return bytes;
    }
    uint16_t rawSize() const { return type_size; }
    DataType *getArrayType() const { return array; }
    bool isVoid() const { return type_size == 0; }
    bool hasCapacity() const { return capacity != UINT16_MAX; }
    uint16_t getCaps() const { return capacity; }


    enum Sizes {
      BYTE = 1,
      WORD = 2,
      DWORD = 4,
      QWORD = 8,
      VOID = 0
    };
};


class IRRet : public IRNode {
  std::unique_ptr<IRNode> value;
public:
  explicit IRRet(std::unique_ptr<IRNode> v);
  IRNode* get() const;
  void set(std::unique_ptr<IRNode> v);
  NodeType type() const override { return NodeType::RET; }
};

class IRFnRef : public IRNode {
  std::string fn_name;
public:
  explicit IRFnRef(std::string name) : fn_name(name) {}
  const std::string& name() const { return fn_name; }
  NodeType type() const override { return NodeType::FN_REF; }
};

class IRLocalRef : public IRNode {
  uint16_t stack_offset;
  DataType *var_type;

public:
  IRLocalRef(uint16_t stack_offset, DataType *type);
  uint16_t offset() const;
  DataType *datatype() const;
  NodeType type() const override { return NodeType::LOCAL_REF; }
};

class IRLocalAddrRef : public IRNode {
  uint16_t stack_offset;
  IRNode *index;
  DataType *var_type;

public:
  IRLocalAddrRef(uint16_t stack_offset, DataType *type, IRNode *index = nullptr);
  uint16_t offset() const;
  IRNode *getIndex() const;
  bool isIndexed() const { return index != nullptr; }
  DataType *datatype() const;
  NodeType type() const override { return NodeType::LADDR_REF; }
};

class IRBody : public IRNode {
  std::vector<std::unique_ptr<IRNode>> statements;
  std::vector<std::string> def_fn_names;

public:
  explicit IRBody(std::vector<std::unique_ptr<IRNode>> s);
  const std::vector<std::unique_ptr<IRNode>>& get() const;
  void Set(int index, std::unique_ptr<IRNode> statement);
  IRBody& operator + (std::unique_ptr<IRNode> statement);
  IRBody& operator += (std::unique_ptr<IRNode> statement);
  NodeType type() const override { return NodeType::BODY; }
  void addDefFn(std::string name) { def_fn_names.push_back(name); }
  bool hasDefFn(std::string name) { return std::find(def_fn_names.begin(), def_fn_names.end(), name) != def_fn_names.end(); }
  std::vector<std::string> getDefFns() { return def_fn_names; }
};

class IRFunction : public IRNode {
public:
  std::string fn_name;
  std::string metadata;
  bool isDefined = true;
  std::vector<std::unique_ptr<IRLocalRef>> fn_locals;
  std::unique_ptr<IRBody> fn_body;
  std::unordered_map<std::string, IRLocalRef*> local_table;
  uint16_t stack_size = 0;
  std::vector<uint16_t> used_offsets;
  FnFlags flags = 0;
  std::vector<DataType*> arg_types;
  bool call_sub = false;
  DataType *return_type;
  bool ignore_stack_abi=false;
  bool canary_needed=false;

public:
  explicit IRFunction(std::string name, std::vector<std::unique_ptr<IRLocalRef>> locals, std::unique_ptr<IRBody> body);
  const std::string& name() const;
  const std::vector<std::unique_ptr<IRLocalRef>>& locals() const;
  IRBody* body() const;
  void SetBody(std::unique_ptr<IRBody> body);
  bool isStack();
  bool isUsed(IRLocalRef *local);
  void occupyOffset(uint16_t offset);
  IRFunction *clone();
  void copyArgTypes(std::vector<DataType*> &types);
  DataType *GetArgType(int index);
  int ArgNum() const { return arg_types.size(); }
  bool isVariadic() const { return flags & FN_VARIADIC; }
  bool isCallSub() const { return call_sub; }

  IRLocalRef *NewLocal(std::string name, DataType *type);
  IRLocalRef* GetLocal(std::string name);


  NodeType type() const override { return NodeType::FUNCTION; }
};

class IRBinOp : public IRNode {
public:
  enum Operation {
    ADD,
    SUB,
    MUL,
    DIV,
    SHL,
    SHR,
    AND,
    EQ,
    LESS,
    GREATER,
    LESSEQ,
    L_ASSIGN,
    G_ASSIGN,
    VA_ASSIGN,
    MOD,
    LOGAND,
    L_PLUS_ASSIGN,
    L_MINUS_ASSIGN,
    G_PLUS_ASSIGN,
    G_MINUS_ASSIGN,
    VA_PLUS_ASSIGN,
    VA_MINUS_ASSIGN
  };

private:
  std::unique_ptr<IRNode> expr_left;
  std::unique_ptr<IRNode> expr_right;
  Operation op;

public:
  IRBinOp(std::unique_ptr<IRNode> l, std::unique_ptr<IRNode> r, Operation o);
  const IRNode* left() const;
  const IRNode* right() const;
  Operation operation() const;
  NodeType type() const override { return NodeType::BIN_OP; }
};

IRBinOp::Operation IRstr2op(std::string str);

class IRLiteral : public IRNode {
  long long value;

public:
  explicit IRLiteral(long long v);
  long long get() const;
  NodeType type() const override { return NodeType::LITERAL; }
};

class IRStringLiteral : public IRNode {
  std::string value;

public:
  explicit IRStringLiteral(std::string v);
  const std::string& get() const;
  NodeType type() const override { return NodeType::STRING; }
};

class IRVariableDecl : public IRNode {
  IRLocalRef *local_ref;
  IRNode *v_value;

public:
  IRVariableDecl(IRLocalRef* local_ref, IRNode* value);
  IRLocalRef *local() const;
  IRNode *value() const;
  NodeType type() const override { return NodeType::LOCAL_DECL; }
};

class IRGlobRef : public IRNode {
  std::string name;
  DataType *g_type;

public:
  IRGlobRef(std::string name, DataType *type);
  const std::string& getName() const;
  DataType *getType() const;
  NodeType type() const override { return NodeType::GLOBAL_REF; }
};

class IRGlobalDecl : public IRNode {
  IRGlobRef *glob_ref;
  IRNode *g_value;

public:
  IRGlobalDecl(IRGlobRef* glob_ref, IRNode* value);
  IRGlobRef *global() const;
  IRNode *value() const;
  NodeType type() const override { return NodeType::GLOBAL_DECL; }
};

class IRArgDecl : public IRNode {
  IRLocalRef *local_ref;

public:
  IRArgDecl(IRLocalRef* local_ref);
  IRLocalRef *local() const;
  NodeType type() const override { return NodeType::ARG_DECL; }
};

class IRFnCall : public IRNode {
  std::string fn_name;
  std::vector<std::unique_ptr<IRNode>> fn_args;
  IRFunction *ref;

public:
  IRFnCall(std::string name, std::vector<std::unique_ptr<IRNode>> args, IRFunction *ref);
  const std::string& name() const;
  const std::vector<std::unique_ptr<IRNode>>& args() const;
  void push_arg(std::unique_ptr<IRNode> arg);
  void replaceArg(int index, std::unique_ptr<IRNode> arg);
  IRFunction *getRef() const;
  NodeType type() const override { return NodeType::FUNCTION_CALL; }
};


class IRInlineAsm : public IRNode {
  std::string asm_code;

public:
  explicit IRInlineAsm(std::string code);
  const std::string& code() const;
  NodeType type() const override { return NodeType::IN_ASM; }
};

struct IRBranch {
  std::unique_ptr<IRNode> condition;
  std::unique_ptr<IRNode> body;
};

class IRBranching : public IRNode {
  std::vector<IRBranch> branches;
  IRBody *else_branch;

public:
  explicit IRBranching(std::vector<IRBranch> &branches);
  const std::vector<IRBranch>& getBranches() const;
  IRBody* getElseBranch() const;
  void setElseBranch(IRBody *body);
  NodeType type() const override { return NodeType::BRANCH; }
};

class IRLooping : public IRNode {
  IRNode *condition;
  IRBody *body;

public:
  IRNode* getCondition() const;
  void setCondition(IRNode* c);
  IRBody* getBody() const;
  void setBody(IRBody *body);
  NodeType type() const override { return NodeType::LOOP; }
};

class IRBreak : public IRNode {
public:
  NodeType type() const override { return NodeType::BREAK; }
};

class IRContinue : public IRNode {
public:
  NodeType type() const override { return NodeType::CONTINUE; }
};

#endif // IR_H