#pragma once

#include "tokenizer.h"
#include "expression.h"

#include <memory>
#include <functional>

class Expression;

class SubroutineCall {
public:
  enum class Type {
    DIRECT,
    INDIRECT
  };

  SubroutineCall(Type type);
  virtual ~SubroutineCall() = default;

  virtual std::unique_ptr<SubroutineCall> clone() const = 0;

  virtual void toXML(std::fstream &outputFile, const std::string &indentation = "") const = 0;
  virtual Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const = 0;

protected:
  Type type_;
};

class DirectSubroutineCall : public SubroutineCall {
public:
  DirectSubroutineCall(const Token &name, const Token &parameterOpen,
      const std::vector< std::unique_ptr<Expression> > &expressionList, const Token &parameterClose);

  std::unique_ptr<SubroutineCall> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token name_;
  Token parameterOpen_;
  std::vector< std::unique_ptr<Expression> > expressionList_;
  Token parameterClose_;
};

class IndirectSubroutineCall : public SubroutineCall {
public:
  IndirectSubroutineCall(const Token &className,
      const Token &dotSymbol, const Token &subroutineName, const Token &parameterOpen,
      const std::vector< std::unique_ptr<Expression> > &expressionList, const Token &parameterClose);

  std::unique_ptr<SubroutineCall> clone() const final;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const final;
  Node::SymbolTable toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const final;

private:
  Token className_;
  Token dotSymbol_;
  Token subroutineName_;
  Token parameterOpen_;
  std::vector< std::unique_ptr<Expression> > expressionList_;
  Token parameterClose_;
};

