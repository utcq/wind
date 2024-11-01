#include <string>
#include <wind/processing/lexer.h>
#include <sstream>
#ifndef PARSER_REP_H
#define PARSER_REP_H

class ParserReport {
public:
  enum Type {
    PARSER_ERROR,
    PARSER_WARNING
  };

  ParserReport(std::string src) : src(src) {}
  void Report(
    ParserReport::Type type,
    Token *expecting=nullptr,
    Token *found=nullptr
  );
private:
  std::istringstream src;
  std::string line(uint16_t line);
};

#endif