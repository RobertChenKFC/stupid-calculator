#include "letnode.h"

LetNode::LetNode(Type type) :
  Node(Node::Type::LET),
  letNodeType_(type)
{

}

VariableLetNode::VariableLetNode(const Token &keyword, const Token &name,
    const Token &eqSymbol, const std::unique_ptr<Expression> &expression, const Token &close) :
  LetNode(LetNode::Type::VARIABLE),
  keyword_(keyword),
  name_(name),
  eqSymbol_(eqSymbol),
  expression_(expression->clone()),
  close_(close)
{

}

std::unique_ptr<Node> VariableLetNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<VariableLetNode>(
      keyword_, name_, eqSymbol_, expression_, close_);
  ret->addAll(children_);
  return ret;
}

void VariableLetNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<letStatement>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  name_.toXML(outputFile, nextIndentation);
  eqSymbol_.toXML(outputFile, nextIndentation);
  expression_->toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</letStatement>" << std::endl;
}

Node::SymbolTable VariableLetNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  expression_->toVMCommands(vmCommands, symbolTable);

  // Local variable
  auto it = symbolTable.localTable.find(name_.val()), end = symbolTable.localTable.end();
  if (it != end) {
    vmCommands.add(VMCommand(
          VMCommand::Operation::POP, VMCommand::Segment::LOCAL,
          static_cast<Word>(it->second.idx)));
  } else {
    // Argument variable
    it = symbolTable.argumentTable.find(name_.val()), end = symbolTable.argumentTable.end();
    if (it != end) {
      vmCommands.add(VMCommand(
            VMCommand::Operation::POP, VMCommand::Segment::ARGUMENT,
            static_cast<Word>(it->second.idx)));
    } else {
      // Field variable
      it = symbolTable.fieldTable.find(name_.val()), end = symbolTable.fieldTable.end();
      if (it != end) {
        vmCommands.add(VMCommand(
              VMCommand::Operation::POP, VMCommand::Segment::THIS,
              static_cast<Word>(it->second.idx)));
      } else {
        // Static variable
        it = symbolTable.staticTable.find(name_.val()), end = symbolTable.staticTable.end();
        if (it != end) {
          vmCommands.add(VMCommand(
                VMCommand::Operation::POP, VMCommand::Segment::STATIC,
                static_cast<Word>(it->second.idx)));
        } else {
          throw std::runtime_error("Line " + std::to_string(name_.lineNumber()) +
              ": Identifier \"" + name_.val() + "\" is not defined");
        }
      }
    }
  }

  return symbolTable;
}

ArrayLetNode::ArrayLetNode(const Token &keyword, const Token &name,
    const Token &idxOpen, const std::unique_ptr<Expression> &idx, const Token &idxClose,
    const Token &eqSymbol, const std::unique_ptr<Expression> &expression, const Token &close) :
  LetNode(LetNode::Type::ARRAY),
  keyword_(keyword),
  name_(name),
  idxOpen_(idxOpen),
  idx_(idx->clone()),
  idxClose_(idxClose),
  eqSymbol_(eqSymbol),
  expression_(expression->clone()),
  close_(close)
{
}

std::unique_ptr<Node> ArrayLetNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<ArrayLetNode>(
      keyword_, name_, idxOpen_, idx_, idxClose_, eqSymbol_, expression_, close_);
  ret->addAll(children_);
  return ret;
}

void ArrayLetNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<letStatement>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  name_.toXML(outputFile, nextIndentation);
  idxOpen_.toXML(outputFile, nextIndentation);
  idx_->toXML(outputFile, nextIndentation);
  idxClose_.toXML(outputFile, nextIndentation);
  eqSymbol_.toXML(outputFile, nextIndentation);
  expression_->toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</letStatement>" << std::endl;
}

Node::SymbolTable ArrayLetNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  expression_->toVMCommands(vmCommands, symbolTable);

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

  vmCommands.add(VMCommand(VMCommand::Operation::POP, VMCommand::Segment::THAT, 0));

  return symbolTable;
}

