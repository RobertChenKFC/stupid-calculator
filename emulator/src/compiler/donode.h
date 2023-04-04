#pragma once

#include "tokenizer.h"
#include "node.h"
#include "subroutinecall.h"
#include "expression.h"

class DoNode : public Node {
public:
  DoNode(const Token &keyword,
      const std::unique_ptr<SubroutineCall> &subroutineCall, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  std::unique_ptr<SubroutineCall> subroutineCall_;
  Token close_;
};

