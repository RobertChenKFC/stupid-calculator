#include "donode.h"

DoNode::DoNode(const Token &keyword,
    const std::unique_ptr<SubroutineCall> &subroutineCall, const Token &close) :
  Node(Node::Type::DO),
  keyword_(keyword),
  subroutineCall_(subroutineCall->clone()),
  close_(close)
{

}

std::unique_ptr<Node> DoNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<DoNode>(keyword_, subroutineCall_, close_);
  ret->addAll(children_);
  return ret;
}

void DoNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<doStatement>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  subroutineCall_->toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</doStatement>" << std::endl;
}

Node::SymbolTable DoNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  subroutineCall_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::POP, VMCommand::Segment::TEMP, 0));

  return symbolTable;
}

