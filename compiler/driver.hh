#ifndef DRIVER_HH
#define DRIVER_HH

#include <string>
#include <unordered_map>
#include "parser.hh"

#define YY_DECL yy::parser::symbol_type yylex(Driver &driver)
YY_DECL;

class Driver {
public:
  std::string parse(const std::string &filename);
  void scanBegin();
  void scanEnd();

  void clear();
  void indent();
  void unindent();
  void addConstant(const std::string &name, int val);
  void addVar(const std::string &name);
  void enterFunction(const std::string &name);
  void exitFunction();
  void initConstants();
  void addLine(const std::string &line, bool newline = true, bool indent = true);
  std::string getCode() const;

  yy::location location;

private:
  std::string filename_;
  std::unordered_map<std::string, int> globalConstants_;
  std::unordered_map<std::string, int> globalArrays_;
  std::unordered_map<std::string, int> localConstants_;
  std::unordered_map<std::string, int> localArrays_;
  bool inFunction_;
  bool inMainFunction_;
  int indent_;
  std::string code_;
};

#endif // DRIVER_HH
