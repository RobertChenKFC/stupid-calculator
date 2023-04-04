#pragma once

#include "tokenizer.h"
#include "node.h"
#include "expression.h"

class LetNode : public Node {
public:
  enum class Type {
    VARIABLE,
    ARRAY
  };

  LetNode(Type type);

protected:
  Type letNodeType_;
};

class VariableLetNode : public LetNode {
public:
  VariableLetNode(const Token &keyword, const Token &name,
      const Token &eqSymbol, const std::unique_ptr<Expression> &expression, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token name_;
  Token eqSymbol_;
  std::unique_ptr<Expression> expression_;
  Token close_;
};

class ArrayLetNode : public LetNode {
public:
  ArrayLetNode(const Token &keyword, const Token &name,
      const Token &idxOpen, const std::unique_ptr<Expression> &idx, const Token &idxClose,
      const Token &eqSymbol, const std::unique_ptr<Expression> &expression, const Token &close);

  std::unique_ptr<Node> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token keyword_;
  Token name_;
  Token idxOpen_;
  std::unique_ptr<Expression> idx_;
  Token idxClose_;
  Token eqSymbol_;
  std::unique_ptr<Expression> expression_;
  Token close_;
};

