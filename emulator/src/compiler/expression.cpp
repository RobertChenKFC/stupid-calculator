#include "expression.h"

#include <iostream>

Expression::Expression(Type type) :
  type_(type)
{

}

UnaryExpression::UnaryExpression(const std::unique_ptr<Term> &term) :
  Expression(Expression::Type::UNARY),
  term_(term->clone())
{

}

std::unique_ptr<Expression> UnaryExpression::clone() const {
  return std::make_unique<UnaryExpression>(term_);
}

void UnaryExpression::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<expression>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  term_->toXML(outputFile, nextIndentation);

  outputFile << indentation << "</expression>" << std::endl;
}

Node::SymbolTable UnaryExpression::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  term_->toVMCommands(vmCommands, symbolTable);

  return symbolTable;
}

void UnaryExpression::print() const {
  std::cout << "printing unary expression" << std::endl;
}

BinaryExpression::BinaryExpression(const std::unique_ptr<Term> &term1,
    const Token &op, const std::unique_ptr<Term> &term2) :
  Expression(Expression::Type::BINARY),
  term1_(term1->clone()),
  op_(op),
  term2_(term2->clone())
{

}

std::unique_ptr<Expression> BinaryExpression::clone() const {
  return std::make_unique<BinaryExpression>(term1_, op_, term2_);
}

void BinaryExpression::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<expression>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  term1_->toXML(outputFile, nextIndentation);
  op_.toXML(outputFile, nextIndentation);
  term2_->toXML(outputFile, nextIndentation);

  outputFile << indentation << "</expression>" << std::endl;
}

Node::SymbolTable BinaryExpression::toVMCommands(VMCommands &vmCommands,
    Node::SymbolTable symbolTable) const {
  term1_->toVMCommands(vmCommands, symbolTable);
  term2_->toVMCommands(vmCommands, symbolTable);

  if (op_.isSymbol('+')) {
    vmCommands.add(VMCommand(VMCommand::Operation::ADD));
  } else if (op_.isSymbol('-')) {
    vmCommands.add(VMCommand(VMCommand::Operation::SUB));
  } else if (op_.isSymbol('*')) {
    vmCommands.add(VMCommand(VMCommand::Operation::CALL, "Math.multiply", 2));
  } else if (op_.isSymbol('/')) {
    vmCommands.add(VMCommand(VMCommand::Operation::CALL, "Math.divide", 2));
  } else if (op_.isSymbol('&')) {
    vmCommands.add(VMCommand(VMCommand::Operation::AND));
  } else if (op_.isSymbol('|')) {
    vmCommands.add(VMCommand(VMCommand::Operation::OR));
  } else if (op_.isSymbol('<')) {
    vmCommands.add(VMCommand(VMCommand::Operation::LT));
  } else if (op_.isSymbol('=')) {
    vmCommands.add(VMCommand(VMCommand::Operation::EQ));
  } else if (op_.isSymbol('>')) {
    vmCommands.add(VMCommand(VMCommand::Operation::GT));
  }

  return symbolTable;
}

void BinaryExpression::print() const {
  std::cout << "Printing binary expression" << std::endl;
}

