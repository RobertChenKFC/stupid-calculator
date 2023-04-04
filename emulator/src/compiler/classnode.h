#pragma once

#include "tokenizer.h"
#include "node.h"

class ClassNode : public Node {
public:
  ClassNode(const Token &keyword, const Token &name, const Token &open, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token name_;
  Token open_;
  Token close_;
};

