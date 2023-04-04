#pragma once

#include "tokenizer.h"
#include "node.h"
#include "expression.h"

class WhileNode : public Node {
public:
  WhileNode(const Token &keyword, const Token &expressionOpen,
      const std::unique_ptr<Expression> &expression, const Token &expressionClose,
      const Token &open, const Token &close);

  static void reset();

  bool hasReturnStatement() const final;

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  static std::size_t labelIdx_;

  Token keyword_;
  Token expressionOpen_;
  std::unique_ptr<Expression> expression_;
  Token expressionClose_;
  Token open_;
  Token close_;
};

