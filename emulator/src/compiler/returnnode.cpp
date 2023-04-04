#include "returnnode.h"

ReturnNode::ReturnNode(Type type) :
  Node(Node::Type::RETURN),
  returnNodeType_(type)
{

}

bool ReturnNode::hasReturnStatement() const {
  return true;
}

ExpressionReturnNode::ExpressionReturnNode(const Token &keyword,
    const std::unique_ptr<Expression> &expression, const Token &close) :
  ReturnNode(ReturnNode::Type::EXPRESSION),
  keyword_(keyword),
  expression_(expression->clone()),
  close_(close)
{

}

std::unique_ptr<Node> ExpressionReturnNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<ExpressionReturnNode>(
      keyword_, expression_, close_);
  ret->addAll(children_);
  return ret;
}

void ExpressionReturnNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<returnStatement>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  expression_->toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</returnStatement>" << std::endl;
}

Node::SymbolTable ExpressionReturnNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  expression_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::RETURN));

  return symbolTable;
}

ExpressionlessReturnNode::ExpressionlessReturnNode(const Token &keyword, const Token &close) :
  ReturnNode(ReturnNode::Type::EXPRESSIONLESS),
  keyword_(keyword),
  close_(close)
{

}

std::unique_ptr<Node> ExpressionlessReturnNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<ExpressionlessReturnNode>(
      keyword_, close_);
  ret->addAll(children_);
  return ret;
}

void ExpressionlessReturnNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<returnStatement>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  outputFile << indentation << "</returnStatement>" << std::endl;
}

Node::SymbolTable ExpressionlessReturnNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  vmCommands.add(VMCommand(VMCommand::Operation::PUSH, VMCommand::Segment::CONSTANT, 0));
  vmCommands.add(VMCommand(VMCommand::Operation::RETURN));

  return symbolTable;
}
