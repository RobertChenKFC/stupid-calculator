#pragma once

#include "node.h"
#include "term.h"

#include <memory>

class Term;

class Expression {
public:
  enum class Type {
    UNARY,
    BINARY
  };

  Expression(Type type);
  virtual ~Expression() = default;

  virtual std::unique_ptr<Expression> clone() const = 0;

  virtual void toXML(std::fstream &outputFile, const std::string &indentation = "") const = 0;
  virtual Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const = 0;
  virtual void print() const = 0;

protected:
  Type type_;
};

class UnaryExpression : public Expression {
public:
  UnaryExpression(const std::unique_ptr<Term> &term);

  std::unique_ptr<Expression> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;
  void print() const final;

private:
  std::unique_ptr<Term> term_;
};

class BinaryExpression : public Expression {
public:
  BinaryExpression(const std::unique_ptr<Term> &term1,
      const Token &op, const std::unique_ptr<Term> &term2);

  std::unique_ptr<Expression> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;
  void print() const final;

private:
  std::unique_ptr<Term> term1_;
  Token op_;
  std::unique_ptr<Term> term2_;
};

