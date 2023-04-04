#include "subroutinenode.h"

#include <iostream>

SubroutineNode::SubroutineNode(const Token &keyword, const Token &returnType, const Token &name,
    const Token &parameterOpen, const std::vector<Token> &parameters, const Token &parameterClose,
    const Token &open, const Token &close) :
  Node(Node::Type::SUBROUTINE),
  keyword_(keyword),
  returnType_(returnType),
  name_(name),
  parameterOpen_(parameterOpen),
  parameters_(parameters),
  parameterClose_(parameterClose),
  open_(open),
  close_(close)
{

}

std::unique_ptr<Node> SubroutineNode::clone() const {
  std::unique_ptr<Node> ret =
    std::make_unique<SubroutineNode>(keyword_, returnType_, name_,
          parameterOpen_, parameters_, parameterClose_, open_, close_);
  ret->addAll(children_);
  return ret;
}

void SubroutineNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<subroutineDec>" << std::endl;

  std::string indentation1 = indentation + "  ";
  keyword_.toXML(outputFile, indentation1);
  returnType_.toXML(outputFile, indentation1);
  name_.toXML(outputFile, indentation1);

  parameterOpen_.toXML(outputFile, indentation1);
  outputFile << indentation1 << "<parameterList>" << std::endl;
  std::string indentation2 = indentation1 + "  ";
  for (const Token &parameter : parameters_) parameter.toXML(outputFile, indentation2);
  outputFile << indentation1 << "</parameterList>" << std::endl;
  parameterClose_.toXML(outputFile, indentation1);

  outputFile << indentation1 << "<subroutineBody>" << std::endl;
  open_.toXML(outputFile, indentation2);
  bool printedStatement = false;
  std::string indentation3 = indentation2 + "  ";
  for (const auto &node : children_) {
    if (!printedStatement && node->isStatement()) {
      outputFile << indentation2 << "<statements>" << std::endl;
      printedStatement = true;
    }
    if (printedStatement) node->toXML(outputFile, indentation3);
    else node->toXML(outputFile, indentation2);
  }
  if (printedStatement) outputFile << indentation2 << "</statements>" << std::endl;
  close_.toXML(outputFile, indentation2);
  outputFile << indentation1 << "</subroutineBody>" << std::endl;

  outputFile << indentation << "</subroutineDec>" << std::endl;
}

Node::SymbolTable SubroutineNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  auto it = children_.begin(), end = children_.end();
  while (it != end && (*it)->isDeclaration()) {
    symbolTable = (*it)->toVMCommands(vmCommands, symbolTable);
    ++it;
  }

  bool isMethod = keyword_.isKeyword("method");
  std::size_t nArgs = isMethod;
  for (auto paramIt = parameters_.begin(), end = parameters_.end(); paramIt < end; paramIt += 3) {
    const Token &name = *(paramIt + 1);
    const Token &type = *paramIt;
    Token kind(name_.lineNumber(), "argument", Token::Type::ARGUMENT);
    symbolTable.argumentTable[name.val()] = Node::TableElement(type, kind, nArgs);
    ++nArgs;

    // To make MSVC happy
    if (paramIt + 2 >= end)
      break;
  }

  std::size_t nVars = symbolTable.localTable.size();
  vmCommands.add(VMCommand(
        VMCommand::Operation::FUNCTION, symbolTable.className + "." + name_.val(),
        static_cast<Word>(nVars)));
  if (isMethod) {
    vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::ARGUMENT, 0));
    vmCommands.add(VMCommand(VMCommand::Operation::POP, VMCommand::Segment::POINTER, 0));
  } else if (keyword_.isKeyword("constructor")) {
    vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT,
          static_cast<Word>(symbolTable.fieldTable.size())));
    vmCommands.add(VMCommand(VMCommand::Operation::CALL, "Memory.alloc", 1));
    vmCommands.add(VMCommand(VMCommand::Operation::POP, VMCommand::Segment::POINTER, 0));
  }

  bool hasReturnStatement = false;
  while (it != end) {
    (*it)->toVMCommands(vmCommands, symbolTable);
    if ((*it)->hasReturnStatement())
      hasReturnStatement = true;
    ++it;
  }

  if (!hasReturnStatement)
    throw std::runtime_error("Subroutine \"" + name_.val() +
        "\": Program flow may reach end of subroutine without \"return\"");

  return symbolTable;
}

