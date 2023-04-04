#include "classnode.h"

ClassNode::ClassNode(const Token &keyword, const Token &name, const Token &open, const Token &close) :
  Node(Node::Type::CLASS),
  keyword_(keyword),
  name_(name),
  open_(open),
  close_(close)
{

}

std::unique_ptr<Node> ClassNode::clone() const {
  std::unique_ptr<Node> ret =
    std::make_unique<ClassNode>(keyword_, name_, open_, close_);
  ret->addAll(children_);
  return ret;
}

void ClassNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<class>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  name_.toXML(outputFile, nextIndentation);

  open_.toXML(outputFile, nextIndentation);
  for (const auto &node : children_)
    node->toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</class>" << std::endl;
}

Node::SymbolTable ClassNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  symbolTable.className = name_.val();
  for (const auto &node : children_) {
    const SymbolTable &newSymbolTable = node->toVMCommands(vmCommands, symbolTable);
    if (node->isDeclaration()) symbolTable = newSymbolTable;
  }

  return symbolTable;
}

