#include "term.h"

Term::Term(Type type) :
  type_(type)
{

}

SingleTerm::SingleTerm(const Token &term) :
  Term(Term::Type::SINGLE),
  term_(term)
{

}

std::unique_ptr<Term> SingleTerm::clone() const {
  return std::make_unique<SingleTerm>(term_);
}

void SingleTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  term_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable SingleTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  if (term_.isIntegerConstant()) {
    vmCommands.add(VMCommand(
          VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT,
          static_cast<Word>(std::stoi(term_.val()))));
  } else if (term_.isStringConstant()) {
    const std::string str = term_.val();
    vmCommands.add(VMCommand(
          VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT,
          static_cast<Word>(str.length())));
    vmCommands.add(VMCommand(VMCommand::Operation::CALL, "String.new", 1));
    for (char c : str) {
      vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT,
            static_cast<Word>(c)));
      vmCommands.add(VMCommand(VMCommand::Operation::CALL, "String.appendChar", 2));
    }
  } else if (term_.isKeyword("true")) {
    vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT, 1));
    vmCommands.add(VMCommand(VMCommand::Operation::NEG));
  } else if (term_.isKeyword("false") || term_.isKeyword("null")) {
    vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT, 0));
  } else if (term_.isKeyword("this")) {
    vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::POINTER, 0));
  } else if (term_.isIdentifier()) {
    // Local variable
    auto it = symbolTable.localTable.find(term_.val()), end = symbolTable.localTable.end();
    if (it != end) {
      vmCommands.add(VMCommand(
            VMCommand::Operation::PUSH, VMCommand::Segment::LOCAL,
            static_cast<Word>(it->second.idx)));
    } else {
      // Argument variable
      it = symbolTable.argumentTable.find(term_.val()), end = symbolTable.argumentTable.end();
      if (it != end) {
        vmCommands.add(VMCommand(
              VMCommand::Operation::PUSH, VMCommand::Segment::ARGUMENT,
              static_cast<Word>(it->second.idx)));
      } else {
        // Field variable
        it = symbolTable.fieldTable.find(term_.val()), end = symbolTable.fieldTable.end();
        if (it != end) {
          vmCommands.add(VMCommand(
                VMCommand::Operation::PUSH, VMCommand::Segment::THIS,
                static_cast<Word>(it->second.idx)));
        } else {
          // Static variable
          it = symbolTable.staticTable.find(term_.val()), end = symbolTable.staticTable.end();
          if (it != end) {
            vmCommands.add(VMCommand(
                  VMCommand::Operation::PUSH, VMCommand::Segment::STATIC,
                  static_cast<Word>(it->second.idx)));
          } else {
            throw std::runtime_error("Line " + std::to_string(term_.lineNumber()) +
                ": Identifier \"" + term_.val() + "\" is not defined");
          }
        }
      }
    }
  }

  return symbolTable;
}

ArrayElementTerm::ArrayElementTerm(const Token &name, const Token &idxOpen,
    const std::unique_ptr<Expression> &idx, const Token &idxClose) :
  Term(Term::Type::ARRAY_ELEMENT),
  name_(name),
  idxOpen_(idxOpen),
  idx_(idx->clone()),
  idxClose_(idxClose)
{

}

std::unique_ptr<Term> ArrayElementTerm::clone() const {
  return std::make_unique<ArrayElementTerm>(
      name_, idxOpen_, idx_, idxClose_);
}

void ArrayElementTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  name_.toXML(outputFile, nextIndentation);
  idxOpen_.toXML(outputFile, nextIndentation);
  idx_->toXML(outputFile, nextIndentation);
  idxClose_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable ArrayElementTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  // Local variable
  auto it = symbolTable.localTable.find(name_.val()), end = symbolTable.localTable.end();
  if (it != end) {
    vmCommands.add(VMCommand(
          VMCommand::Operation::PUSH, VMCommand::Segment::LOCAL,
          static_cast<Word>(it->second.idx)));
  } else {
    // Argument variable
    it = symbolTable.argumentTable.find(name_.val()), end = symbolTable.argumentTable.end();
    if (it != end) {
      vmCommands.add(VMCommand(
            VMCommand::Operation::PUSH, VMCommand::Segment::ARGUMENT,
            static_cast<Word>(it->second.idx)));
    } else {
      // Field variable
      it = symbolTable.fieldTable.find(name_.val()), end = symbolTable.fieldTable.end();
      if (it != end) {
        vmCommands.add(VMCommand(
              VMCommand::Operation::PUSH, VMCommand::Segment::THIS,
              static_cast<Word>(it->second.idx)));
      } else {
        // Static variable
        it = symbolTable.staticTable.find(name_.val()), end = symbolTable.staticTable.end();
        if (it != end) {
          vmCommands.add(VMCommand(
                VMCommand::Operation::PUSH, VMCommand::Segment::STATIC,
                static_cast<Word>(it->second.idx)));
        } else {
          throw std::runtime_error("Line " + std::to_string(name_.lineNumber()) +
              ": Identifier \"" + name_.val() + "\" is not defined");
        }
      }
    }
  }

  idx_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::ADD));
  vmCommands.add(VMCommand(VMCommand::Operation::POP, VMCommand::Segment::POINTER, 1));
  vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::THAT, 0));

  return symbolTable;
}

SubroutineCallTerm::SubroutineCallTerm(const std::unique_ptr<SubroutineCall> &subroutineCall) :
  Term(Term::Type::SUBROUTINE_CALL),
  subroutineCall_(subroutineCall->clone())
{

}

std::unique_ptr<Term> SubroutineCallTerm::clone() const {
  return std::make_unique<SubroutineCallTerm>(
      subroutineCall_);
}

void SubroutineCallTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  subroutineCall_->toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable SubroutineCallTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  subroutineCall_->toVMCommands(vmCommands, symbolTable);

  return symbolTable;
}

BracketedExpressionTerm::BracketedExpressionTerm(const Token &expressionOpen,
    const std::unique_ptr<Expression> &expression, const Token &expressionClose) :
  Term(Term::Type::EXPRESSION),
  expressionOpen_(expressionOpen),
  expression_(expression->clone()),
  expressionClose_(expressionClose)
{

}

std::unique_ptr<Term> BracketedExpressionTerm::clone() const {
  return std::make_unique<BracketedExpressionTerm>(
      expressionOpen_, expression_, expressionClose_);
}

void BracketedExpressionTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  expressionOpen_.toXML(outputFile, nextIndentation);
  expression_->toXML(outputFile, nextIndentation);
  expressionClose_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable BracketedExpressionTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  expression_->toVMCommands(vmCommands, symbolTable);

  return symbolTable;
}

ExpressionTerm::ExpressionTerm(const std::unique_ptr<Expression> &expression) :
  Term(Term::Type::EXPRESSION),
  expression_(expression->clone())
{

}

std::unique_ptr<Term> ExpressionTerm::clone() const {
  return std::make_unique<ExpressionTerm>(expression_);
}

void ExpressionTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  expression_->toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable ExpressionTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  expression_->toVMCommands(vmCommands, symbolTable);

  return symbolTable;
}

UnaryOpTerm::UnaryOpTerm(const Token &unaryOp, const std::unique_ptr<Term> &term) :
  Term(Term::Type::UNARY_OP),
  unaryOp_(unaryOp),
  term_(term->clone())
{

}

std::unique_ptr<Term> UnaryOpTerm::clone() const {
  return std::make_unique<UnaryOpTerm>(unaryOp_, term_);
}

void UnaryOpTerm::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<term>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  unaryOp_.toXML(outputFile, nextIndentation);
  term_->toXML(outputFile, nextIndentation);

  outputFile << indentation << "</term>" << std::endl;
}

Node::SymbolTable UnaryOpTerm::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  term_->toVMCommands(vmCommands, symbolTable);
  if (unaryOp_.isSymbol('-')) vmCommands.add(VMCommand(VMCommand::Operation::NEG));
  else if (unaryOp_.isSymbol('~')) vmCommands.add(VMCommand(VMCommand::Operation::NOT));

  return symbolTable;
}
