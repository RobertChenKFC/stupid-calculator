#include "ifnode.h"

std::size_t IfNode::labelIdx_ = 0;

IfNode::IfNode(Type type) :
  Node(Node::Type::IF),
  ifNodeType_(type)
{

}

void IfNode::reset() {
  labelIdx_ = 0;
}

bool IfNode::hasReturnStatement() const {
  for (const auto &node : children_) {
    if (node->hasReturnStatement())
      return true;
  }
  return false;
}

SingleIfNode::SingleIfNode(const Token &keyword, const Token &expressionOpen,
    const std::unique_ptr<Expression> &expression, const Token &expressionClose,
    const Token &open, const Token &close) :
  IfNode(IfNode::Type::SINGLE),
  keyword_(keyword),
  expressionOpen_(expressionOpen),
  expression_(expression->clone()),
  expressionClose_(expressionClose),
  open_(open),
  close_(close)
{

}

std::unique_ptr<Node> SingleIfNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<SingleIfNode>(
      keyword_, expressionOpen_, expression_, expressionClose_, open_, close_);
  ret->addAll(children_);
  return ret;
}

void SingleIfNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<ifStatement>" << std::endl;

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

  outputFile << indentation << "</ifStatement>" << std::endl;
}

Node::SymbolTable SingleIfNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  std::size_t curLabelIdx = labelIdx_;
  ++labelIdx_;

  expression_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::IF_GOTO, "IF" + std::to_string(curLabelIdx)));
  vmCommands.add(VMCommand(VMCommand::Operation::GOTO, "END_IF" + std::to_string(curLabelIdx)));
  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "IF" + std::to_string(curLabelIdx)));
  for (const auto &node : children_)
    node->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "END_IF" + std::to_string(curLabelIdx)));

  return symbolTable;
}

IfElseIfNode::IfElseIfNode(const Token &ifKeyword, const Token &expressionOpen,
    const std::unique_ptr<Expression> &expression, const Token &expressionClose,
    const Token &ifOpen, const Token &ifClose,
    std::size_t elseBegin, const Token &elseKeyword,
    const Token &elseOpen, const Token &elseClose) :
  IfNode(IfNode::Type::IF_ELSE),
  ifKeyword_(ifKeyword),
  expressionOpen_(expressionOpen),
  expression_(expression->clone()),
  expressionClose_(expressionClose),
  ifOpen_(ifOpen),
  ifClose_(ifClose),
  elseBegin_(elseBegin),
  elseKeyword_(elseKeyword),
  elseOpen_(elseOpen),
  elseClose_(elseClose)
{

}

std::unique_ptr<Node> IfElseIfNode::clone() const {
  std::unique_ptr<Node> ret = std::make_unique<IfElseIfNode>(
      ifKeyword_, expressionOpen_, expression_, expressionClose_,
        ifOpen_, ifClose_, elseBegin_, elseKeyword_, elseOpen_, elseClose_);
  ret->addAll(children_);
  return ret;
}

void IfElseIfNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation << "<ifStatement>" << std::endl;

  std::string indentation1 = indentation + "  ";
  ifKeyword_.toXML(outputFile, indentation1);
  expressionOpen_.toXML(outputFile, indentation1);
  expression_->toXML(outputFile, indentation1);
  expressionClose_.toXML(outputFile, indentation1);
  ifOpen_.toXML(outputFile, indentation1);

  outputFile << indentation1 << "<statements>" << std::endl;
  std::string indentation2 = indentation1 + "  ";
  for (std::size_t i = 0; i < elseBegin_; ++i)
    children_[i]->toXML(outputFile, indentation2);
  outputFile << indentation1 << "</statements>" << std::endl;

  ifClose_.toXML(outputFile, indentation1);
  elseKeyword_.toXML(outputFile, indentation1);
  elseOpen_.toXML(outputFile, indentation1);

  outputFile << indentation1 << "<statements>" << std::endl;
  for (std::size_t i = elseBegin_; i < children_.size(); ++i)
    children_[i]->toXML(outputFile, indentation2);
  outputFile << indentation1 << "</statements>" << std::endl;

  elseClose_.toXML(outputFile, indentation1);

  outputFile << indentation << "</ifStatement>" << std::endl;
}

Node::SymbolTable IfElseIfNode::toVMCommands(VMCommands &vmCommands, Node::SymbolTable symbolTable) const {
  std::size_t curLabelIdx = labelIdx_;
  ++labelIdx_;

  expression_->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::IF_GOTO, "IF" + std::to_string(curLabelIdx)));

  for (std::size_t i = elseBegin_; i < children_.size(); ++i)
    children_[i]->toVMCommands(vmCommands, symbolTable);
  vmCommands.add(VMCommand(VMCommand::Operation::GOTO, "END_IF" + std::to_string(curLabelIdx)));

  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "IF" + std::to_string(curLabelIdx)));
  for (std::size_t i = 0; i < elseBegin_; ++i)
    children_[i]->toVMCommands(vmCommands, symbolTable);

  vmCommands.add(VMCommand(VMCommand::Operation::LABEL, "END_IF" + std::to_string(curLabelIdx)));

  return symbolTable;
}

