#pragma once

#include "tokenizer.h"
#include "node.h"
#include "expression.h"

class ReturnNode : public Node {
public:
  enum class Type {
    EXPRESSION,
    EXPRESSIONLESS
  };

  ReturnNode(Type type);

  bool hasReturnStatement() const final;

protected:
  Type returnNodeType_;
};

class ExpressionReturnNode : public ReturnNode {
public:
  ExpressionReturnNode(const Token &keyword,
      const std::unique_ptr<Expression> &expression, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  std::unique_ptr<Expression> expression_;
  Token close_;
};

class ExpressionlessReturnNode : public ReturnNode {
public:
  ExpressionlessReturnNode(const Token &keyword, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token close_;
};

