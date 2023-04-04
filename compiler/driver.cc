#include "driver.hh"
#include "parser.hh"

std::string Driver::parse(const std::string &filename) {
  clear();

  filename_ = filename;
  location.initialize(&filename_);
  scanBegin();

  yy::parser parser(*this);
  parser.set_debug_level(false);
  parser();

  return getCode();
}

void Driver::clear() {
  indent_ = 0;
  globalConstants_.clear();
  globalArrays_.clear();
  localConstants_.clear();
  localArrays_.clear();
  inFunction_ = false;
}

void Driver::indent() {
  ++indent_;
}

void Driver::unindent() {
  --indent_;
}

void Driver::addConstant(const std::string &name, int val) {
  auto &constants = inFunction_ ? localConstants_ : globalConstants_;
  constants[name] = val;
  addVar(name);
}

void Driver::addVar(const std::string &name) {
  if (inFunction_)
    addLine("var int " + name + ";");
  else
    addLine("static int " + name + ";");
}

void Driver::initConstants() {
  auto init = [&](const std::unordered_map<std::string, int> &constants) {
    for (const auto &[name, val] : constants)
      addLine(std::string("let ") + name + " = " + std::to_string(val) + ";");
  };
  if (inMainFunction_)
    init(globalConstants_);
  init(localConstants_);
}

void Driver::enterFunction(const std::string &name) {
  inFunction_ = true;
  if (name == "main")
    inMainFunction_ = true;
  indent();
}

void Driver::exitFunction() {
  inFunction_ = inMainFunction_ = false;
  localConstants_.clear();
  localArrays_.clear();
  unindent();
}

void Driver::addLine(const std::string &line, bool newline, bool indent) {
  if (indent) {
    for (int i = 0; i < indent_; ++i)
      code_ += "  ";
  }
  code_ += line;
  if (newline)
    code_ += "\n";
}

std::string Driver::getCode() const {
  return code_;
}

