#include "whilenode.h"

std::size_t WhileNode::labelIdx_ = 0;

WhileNode::WhileNode(const Token &keyword, const Token &expressionOpen,
    const std::unique_ptr<Expression> &expression, const Token &expressionClose,
    const Token &open, const Token &close) :
  Node(Node::Type::WHILE),
  keyword_(keyword),
  expressionOpen_(expressionOpen),
  expression_(expression->clone()),
  expressionClose_(expressionClose),
  open_(open),
  close_(close)
{

}

void WhileNode::reset() {
  labelIdx_ = 0;
}

bool WhileNode::hasReturnStatement() const {
  for (const auto &node : children_) {
    if (node->hasReturnStatement())
      return true;
  }
  return false;
}

std::unique_ptr<Node> WhileNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<WhileNode>(
      keyword_, expressionOpen_, expression_, expressionClose_, open_, close_);
  ret->addAll(children_);
  return ret;
}

void WhileNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<whileStatement>" << std::endl;

  std::string indentation1 = indentation + "  ";
  keyword_.toXML(outputFile, indentation1);
  expressionOpen_.toXML(outputFile, indentation1);
  expression_->toXML(outputFile, indentation1);
  expressionClose_.toXML(outputFile, indentation1);
  open_.toXML(outputFile, indentation1);

  outputFile << indentation1 << "<statements>" << std::endl;
  std::string indentation2 = indentation1 + "  ";
  for (const auto &node : children_) node->toXML(outputFile, indentation2);
  outputFile << indentation1 << "</statements>" << std::endl;

  close_.toXML(outputFile, indentation1);

  outputFile << indentation << "</whileStatement>" << std::endl;
}

Node::SymbolTable WhileNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  std::size_t curLabelIdx = labelIdx_;
  ++labelIdx_;

  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "WHILE" + std::to_string(curLabelIdx)));
  expression_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::IF_GOTO, "WHILE_BODY" + std::to_string(curLabelIdx)));
  vmCommands.add(VMCommand(VMCommand::Operation::GOTO, "END_WHILE" + std::to_string(curLabelIdx)));

  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "WHILE_BODY" + std::to_string(curLabelIdx)));
  for (const auto &node : children_)
    node->toVMCommands(vmCommands, symbolTable);

  vmCommands.add(VMCommand(VMCommand::Operation::GOTO, "WHILE" + std::to_string(curLabelIdx)));
  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "END_WHILE" + std::to_string(curLabelIdx)));

  return symbolTable;
}

