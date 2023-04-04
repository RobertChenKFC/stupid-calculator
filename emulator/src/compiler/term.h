#pragma once

#include "subroutinecall.h"
#include "expression.h"

#include <memory>

class SubroutineCall;
class Expression;

class Term {
public:
  enum class Type {
    SINGLE,
    ARRAY_ELEMENT,
    SUBROUTINE_CALL,
    BRACKET_EXPRESSION,
    EXPRESSION,
    UNARY_OP,
  };

  Term(Type type);
  virtual ~Term() = default;

  virtual std::unique_ptr<Term> clone() const = 0;

  virtual void toXML(std::fstream &outputFile, const std::string &indentation = "") const = 0;
  virtual Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const = 0;

protected:
  Type type_;
};

class SingleTerm : public Term {
public:
  SingleTerm(const Token &term);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token term_;
};

class ArrayElementTerm : public Term {
public:
  ArrayElementTerm(const Token &name, const Token &idxOpen,
      const std::unique_ptr<Expression> &idx, const Token &idxClose);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token name_;
  Token idxOpen_;
  std::unique_ptr<Expression> idx_;
  Token idxClose_;
};

class SubroutineCallTerm : public Term {
public:
  SubroutineCallTerm(const std::unique_ptr<SubroutineCall> &subroutineCall);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  std::unique_ptr<SubroutineCall> subroutineCall_;
};

class BracketedExpressionTerm : public Term {
public:
  BracketedExpressionTerm(const Token &expressionOpen,
      const std::unique_ptr<Expression> &expression, const Token &expressionClose);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token expressionOpen_;
  std::unique_ptr<Expression> expression_;
  Token expressionClose_;
};

class ExpressionTerm : public Term {
public:
  ExpressionTerm(const std::unique_ptr<Expression> &expression);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  std::unique_ptr<Expression> expression_;
};

class UnaryOpTerm : public Term {
public:
  UnaryOpTerm(const Token &unaryOp, const std::unique_ptr<Term> &term);

  std::unique_ptr<Term> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token unaryOp_;
  std::unique_ptr<Term> term_;
};

