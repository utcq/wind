#include <string>
#include <unordered_map>
#include <stdint.h>

#ifndef TOKEN_H
#define TOKEN_H

typedef std::pair<u_int16_t, u_int16_t> TokenPos;
typedef std::pair<TokenPos, TokenPos> TokenRange;

typedef uint16_t TokenSrcId;

class Token {
public:
  enum Type {
    IDENTIFIER,
    INTEGER,
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    ASSIGN,
    LPAREN,
    RPAREN,
    COLON,
    LBRACE,
    RBRACE,
    COMMA,
    SEMICOLON,
    LBRACKET,
    RBRACKET,
    AT,
    QMARK,
    STRING,
    VARDC,
    AND,
  };
  
  std::string value;
  Token::Type type;
  std::string name;
  TokenRange range;
  TokenSrcId srcId;

  Token(
    std::string value,
    Token::Type type,
    std::string name,
    TokenRange range,
    TokenSrcId srcId
  ) : value(value), type(type), name(name), range(range), srcId(srcId) {}
};

std::unordered_map<std::string, Token::Type> const SymbolTable = {
  {"+", Token::Type::PLUS},
  {"-", Token::Type::MINUS},
  {"*", Token::Type::MULTIPLY},
  {"/", Token::Type::DIVIDE},
  {"%", Token::Type::MODULO},
  {"=", Token::Type::ASSIGN},
  {"(", Token::Type::LPAREN},
  {")", Token::Type::RPAREN},
  {":", Token::Type::COLON},
  {"{", Token::Type::LBRACE},
  {"}", Token::Type::RBRACE},
  {",", Token::Type::COMMA},
  {";", Token::Type::SEMICOLON},
  {"[", Token::Type::LBRACKET},
  {"]", Token::Type::RBRACKET},
  {"@", Token::Type::AT},
  {"?", Token::Type::QMARK},
  {"...", Token::Type::VARDC},
  {"&", Token::Type::AND}
};

#endif