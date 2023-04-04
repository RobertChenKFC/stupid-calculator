#include "declarationnode.h"

DeclarationNode::DeclarationNode(const Token &keyword, const Token &type,
    const std::vector<Token> &variableList, const Token &close) :
  Node(Node::Type::DECLARATION),
  keyword_(keyword),
  type_(type),
  variableList_(variableList),
  close_(close)
{

}

std::unique_ptr<Node> DeclarationNode::clone() const {
  std::unique_ptr<Node> ret =
    std::make_unique<DeclarationNode>(keyword_, type_, variableList_, close_);
  ret->addAll(children_);
  return ret;
}

void DeclarationNode::toXML(std::fstream &outputFile, const std::string &indentation) const {
  if (keyword_.isKeyword("var")) outputFile << indentation << "<varDec>" << std::endl;
  else outputFile << indentation << "<classVarDec>" << std::endl;

  std::string nextIndentation = indentation + "  ";
  keyword_.toXML(outputFile, nextIndentation);
  type_.toXML(outputFile, nextIndentation);
  for (const auto &variable : variableList_)
    variable.toXML(outputFile, nextIndentation);
  close_.toXML(outputFile, nextIndentation);

  if (keyword_.isKeyword("var")) outputFile << indentation << "</varDec>" << std::endl;
  else outputFile << indentation << "</classVarDec>" << std::endl;
}

Node::SymbolTable DeclarationNode::toVMCommands(VMCommands &/* vmCommands */,
    Node::SymbolTable symbolTable) const {
  bool staticDec = keyword_.isKeyword("static");
  bool fieldDec = keyword_.isKeyword("field");
  for (const Token &token : variableList_) {
    if (token.isIdentifier()) {
      if (staticDec)
        symbolTable.staticTable[token.val()] =
          Node::TableElement(type_, keyword_, symbolTable.staticTable.size());
      else if (fieldDec)
        symbolTable.fieldTable[token.val()] =
          Node::TableElement(type_, keyword_, symbolTable.fieldTable.size());
      else
        symbolTable.localTable[token.val()] =
          Node::TableElement(type_, keyword_, symbolTable.localTable.size());
    }
  }

  return symbolTable;
}

