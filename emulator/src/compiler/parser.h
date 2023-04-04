#pragma once

#include "node.h"
#include "classnode.h"
#include "subroutinenode.h"
#include "declarationnode.h"
#include "letnode.h"
#include "donode.h"
#include "ifnode.h"
#include "whilenode.h"
#include "returnnode.h"

class Parser {
public:
  struct SubroutineCallReturnType {
    std::unique_ptr<SubroutineCall> parsedSubroutineCall;
    std::vector<Token>::iterator endIterator;
  };

  struct TermReturnType {
    std::unique_ptr<Term> parsedTerm;
    std::vector<Token>::iterator endIterator;
  };

  struct ExpressionReturnType {
    std::unique_ptr<Expression> parsedExpression;
    std::vector<Token>::iterator endIterator;
  };

  struct ReturnType {
    std::unique_ptr<Node> parsedNode;
    std::vector<Token>::iterator endIterator;
  };

  Parser(const std::string &inputFileName);
  Parser(const std::string &inputFileName, const std::string &input);

  static void reset();

  Nodes parse() const;
  VMCommands toVMCommands() const;
  void toXML(const std::string &outputFileName) const;
  void toVMCode(const std::string &outputFileName) const;

  SubroutineCallReturnType parseSubroutineCall(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  TermReturnType parseTerm(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ExpressionReturnType parseExpression(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseStatement(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;

  ReturnType parseClassNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseSubroutineNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseDeclarationNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseLetNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseDoNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseIfNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseWhileNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;
  ReturnType parseReturnNode(const std::vector<Token>::iterator &begin,
      const std::vector<Token>::iterator &end) const;

private:
  std::string inputFileName_;
  std::string input_;
};

