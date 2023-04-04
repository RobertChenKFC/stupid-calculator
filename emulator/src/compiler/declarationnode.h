#pragma once

#include "tokenizer.h"
#include "node.h"

class DeclarationNode : public Node {
public:
  DeclarationNode(const Token &keyword, const Token &type,
      const std::vector<Token> &variableList, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token type_;
  std::vector<Token> variableList_;
  Token close_;
};

