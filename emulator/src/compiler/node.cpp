#include "node.h"

#include <iostream>

Node::TableElement::TableElement(const Token &_type, const Token &_kind, std::size_t _idx) :
  type(_type),
  kind(_kind),
  idx(_idx)
{

}

Node::Node(Type type) :
  type_(type)
{

}

bool Node::isDeclaration() const {
  return type_ == Type::DECLARATION;
}

bool Node::isStatement(Type type) const {
  if (type == Node::Type::ANY)
    return type_ == Type::LET || type_ == Type::IF || type_ == Type::WHILE ||
      type_ == Type::DO || type_ == Type::RETURN;
  return type_ == type;
}

bool Node::hasReturnStatement() const {
  return false;
}

void Node::add(const std::unique_ptr<Node> &node) {
  children_.push_back(node->clone());
}

void Node::addAll(const std::vector< std::unique_ptr<Node> > &nodes) {
  children_.reserve(children_.size() + nodes.size());
  for (const auto &node : nodes) children_.push_back(node->clone());
}

void Nodes::add(const std::unique_ptr<Node> &node) {
  nodes_.push_back(node->clone());
}

void Nodes::toXML(std::fstream &outputFile, const std::string &indentation) const {
  for (const auto &node : nodes_) node->toXML(outputFile, indentation);
}

VMCommands Nodes::toVMCommands() const {
  VMCommands vmCommands;
  for (const auto &node : nodes_) node->toVMCommands(vmCommands, Node::SymbolTable());
  return vmCommands;
}

std::vector< std::unique_ptr<Node> >::iterator Nodes::begin() {
  return nodes_.begin();
}

std::vector< std::unique_ptr<Node> >::const_iterator Nodes::begin() const {
  return nodes_.begin();
}

std::vector< std::unique_ptr<Node> >::iterator Nodes::end() {
  return nodes_.end();
}

std::vector< std::unique_ptr<Node> >::const_iterator Nodes::end() const {
  return nodes_.end();
}

std::size_t Nodes::size() const {
  return nodes_.size();
}

