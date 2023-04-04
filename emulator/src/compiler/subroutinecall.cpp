#include "subroutinecall.h"

#include <iostream>

SubroutineCall::SubroutineCall(Type type) :
  type_(type)
{

}

DirectSubroutineCall::DirectSubroutineCall(const Token &name, const Token &parameterOpen,
    const std::vector< std::unique_ptr<Expression> > &expressionList, const Token &parameterClose) :
  SubroutineCall(SubroutineCall::Type::DIRECT),
  expressionList_(expressionList.size())
{
  name_ = name;
  parameterOpen_ = parameterOpen;
  parameterClose_ = parameterClose;

  for (std::size_t i = 0; i < expressionList_.size(); ++i)
    expressionList_[i] = expressionList[i]->clone();
}

std::unique_ptr<SubroutineCall> DirectSubroutineCall::clone() const {
  return std::make_unique<DirectSubroutineCall>(
      name_, parameterOpen_, expressionList_, parameterClose_);
}

void DirectSubroutineCall::toXML(std::fstream &outputFile, const std::string &indentation) const {
  name_.toXML(outputFile, indentation);
  parameterOpen_.toXML(outputFile, indentation);

  outputFile << indentation << "<expressionList>" << std::endl;
  std::string nextIndentation = indentation + "  ";
  bool first = true;
  for (const auto &expression : expressionList_) {
    if (!first) outputFile << nextIndentation << "<symbol> , </symbol>" << std::endl;
    first = false;
    expression->toXML(outputFile, nextIndentation);
  }
  outputFile << indentation << "</expressionList>" << std::endl;

  parameterClose_.toXML(outputFile, indentation);
}

Node::SymbolTable DirectSubroutineCall::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::POINTER, 0));
  for (const auto &expression : expressionList_)
    expression->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::CALL,
        symbolTable.className + "." + name_.val(),
        static_cast<Word>(expressionList_.size()) + 1));

  return symbolTable;
}

IndirectSubroutineCall::IndirectSubroutineCall(const Token &className,
    const Token &dotSymbol, const Token &subroutineName, const Token &parameterOpen,
    const std::vector< std::unique_ptr<Expression> > &expressionList, const Token &parameterClose) :
  SubroutineCall(SubroutineCall::Type::DIRECT),
  // className_(className_),
  // dotSymbol_(dotSymbol),
  // subroutineName_(subroutineName),
  // parameterOpen_(parameterOpen_),
  expressionList_(expressionList.size())
  // parameterClose_(parameterClose)
{
  className_ = className;
  dotSymbol_ = dotSymbol;
  subroutineName_ = subroutineName;
  parameterOpen_ = parameterOpen;
  parameterClose_ = parameterClose;

  for (std::size_t i = 0; i < expressionList_.size(); ++i)
    expressionList_[i] = expressionList[i]->clone();
}

std::unique_ptr<SubroutineCall> IndirectSubroutineCall::clone() const {
  return std::make_unique<IndirectSubroutineCall>(
      className_, dotSymbol_, subroutineName_, parameterOpen_,
        expressionList_, parameterClose_);
}

void IndirectSubroutineCall::toXML(std::fstream &outputFile, const std::string &indentation) const {
  className_.toXML(outputFile, indentation);
  dotSymbol_.toXML(outputFile, indentation);
  subroutineName_.toXML(outputFile, indentation);
  parameterOpen_.toXML(outputFile, indentation);

  outputFile << indentation << "<expressionList>" << std::endl;
  std::string nextIndentation = indentation + "  ";
  bool first = true;
  for (const auto &expression : expressionList_) {
    if (!first) outputFile << nextIndentation << "<symbol> , </symbol>" << std::endl;
    first = false;
    expression->toXML(outputFile, nextIndentation);
  }
  outputFile << indentation << "</expressionList>" << std::endl;

  parameterClose_.toXML(outputFile, indentation);
}

Node::SymbolTable IndirectSubroutineCall::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  // Local object instance
  auto it = symbolTable.localTable.find(className_.val()), end = symbolTable.localTable.end();
  if (it != end) {
    vmCommands.add(VMCommand(
          VMCommand::Operation::PUSH, VMCommand::Segment::LOCAL,
          static_cast<Word>(it->second.idx)));
    for (const auto &expression : expressionList_)
      expression->toVMCommands(vmCommands, symbolTable);

    vmCommands.add(VMCommand(VMCommand::Operation::CALL,
          it->second.type.val() + "." + subroutineName_.val(),
          static_cast<Word>(expressionList_.size()) + 1));
  } else {
    // Argument object instance
    it = symbolTable.argumentTable.find(className_.val()), end = symbolTable.argumentTable.end();
    if (it != end) {
      vmCommands.add(VMCommand(
            VMCommand::Operation::PUSH, VMCommand::Segment::ARGUMENT,
            static_cast<Word>(it->second.idx)));
      for (const auto &expression : expressionList_)
        expression->toVMCommands(vmCommands, symbolTable);

      vmCommands.add(VMCommand(VMCommand::Operation::CALL,
            it->second.type.val() + "." + subroutineName_.val(),
            static_cast<Word>(expressionList_.size()) + 1));
    } else {
      // Field object instance
      it = symbolTable.fieldTable.find(className_.val()), end = symbolTable.fieldTable.end();
      if (it != end) {
        vmCommands.add(VMCommand(
              VMCommand::Operation::PUSH, VMCommand::Segment::THIS,
              static_cast<Word>(it->second.idx)));
        for (const auto &expression : expressionList_)
          expression->toVMCommands(vmCommands, symbolTable);

        vmCommands.add(VMCommand(VMCommand::Operation::CALL,
              it->second.type.val() + "." + subroutineName_.val(),
              static_cast<Word>(expressionList_.size()) + 1));
      } else {
        // Static object instance
        it = symbolTable.staticTable.find(className_.val()), end = symbolTable.staticTable.end();
        if (it != end) {
          vmCommands.add(VMCommand(
                VMCommand::Operation::PUSH, VMCommand::Segment::STATIC,
                static_cast<Word>(it->second.idx)));
          for (const auto &expression : expressionList_)
            expression->toVMCommands(vmCommands, symbolTable);

          vmCommands.add(VMCommand(VMCommand::Operation::CALL,
                it->second.type.val() + "." + subroutineName_.val(),
                static_cast<Word>(expressionList_.size()) + 1));
        } else {
          // Static function call
          for (const auto &expression : expressionList_)
            expression->toVMCommands(vmCommands, symbolTable);

          vmCommands.add(VMCommand(VMCommand::Operation::CALL,
                className_.val() + "." + subroutineName_.val(),
                static_cast<Word>(expressionList_.size())));
        }
      }
    }
  }

  return symbolTable;
}

