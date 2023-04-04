#pragma once

#include "tokenizer.h"
#include "node.h"

class SubroutineNode : public Node {
public:
  SubroutineNode(const Token &keyword, const Token &returnType, const Token &name,
      const Token &parameterOpen, const std::vector<Token> &parameters, const Token &parameterClose,
      const Token &open, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token returnType_;
  Token name_;
  Token parameterOpen_;
  std::vector<Token> parameters_;
  Token parameterClose_;
  Token open_;
  Token close_;
};

