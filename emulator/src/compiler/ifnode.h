#pragma once

#include "tokenizer.h"
#include "node.h"
#include "expression.h"

class IfNode : public Node {
public:
  enum class Type {
    SINGLE,
    IF_ELSE
  };

  IfNode(Type type);

  static void reset();

  bool hasReturnStatement() const final;

protected:
  static std::size_t labelIdx_;

  Type ifNodeType_;
};

class SingleIfNode : public IfNode {
public:
  SingleIfNode(const Token &keyword, const Token &expressionOpen,
      const std::unique_ptr<Expression> &expression, const Token &expressionClose,
      const Token &open, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token expressionOpen_;
  std::unique_ptr<Expression> expression_;
  Token expressionClose_;
  Token open_;
  Token close_;
};

class IfElseIfNode : public IfNode {
public:
  IfElseIfNode(const Token &ifKeyword, const Token &expressionOpen,
      const std::unique_ptr<Expression> &expression, const Token &expressionClose,
      const Token &ifOpen, const Token &ifClose,
      std::size_t elseBegin, const Token &elseKeyword,
      const Token &elseOpen, const Token &elseClose);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token ifKeyword_;
  Token expressionOpen_;
  std::unique_ptr<Expression> expression_;
  Token expressionClose_;
  Token ifOpen_;
  Token ifClose_;
  std::size_t elseBegin_;
  Token elseKeyword_;
  Token elseOpen_;
  Token elseClose_;
};

