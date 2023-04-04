#include "parser.h"

#include <iostream>

Parser::Parser(const std::string &inputFileName) :
  inputFileName_(inputFileName)
{

}

Parser::Parser(const std::string &inputFileName, const std::string &input) :
  inputFileName_(inputFileName),
  input_(input)
{

}

void Parser::reset() {
  IfNode::reset();
  WhileNode::reset();
}

Nodes Parser::parse() const {
  auto tokenizer = input_.empty() ? Tokenizer(inputFileName_) : Tokenizer(inputFileName_, input_);
  Tokens tokens = tokenizer.tokenize();

  std::vector<Token>::iterator it = tokens.begin(), end = tokens.end();
  Nodes nodes;
  const Parser::ReturnType& ret = parseClassNode(it, end);
  nodes.add(ret.parsedNode);
  return nodes;
}

VMCommands Parser::toVMCommands() const {
  Nodes nodes = parse();
  auto vmCommands = nodes.toVMCommands();
  return vmCommands;
}

void Parser::toXML(const std::string &outputFileName) const {
  std::fstream outputFile(outputFileName, std::ios::out);

  const Nodes &nodes = parse();
  nodes.toXML(outputFile);

  outputFile.close();
}

void Parser::toVMCode(const std::string &outputFileName) const {
  std::fstream outputFile(outputFileName, std::ios::out);

  toVMCommands().toVMCode(outputFile);

  outputFile.close();
}

Parser::SubroutineCallReturnType Parser::parseSubroutineCall(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  if (begin == end) throw std::runtime_error("Incomplete subroutine call");

  if (begin + 1 != end && (begin + 1)->isSymbol('(')) {
    if (!begin->isIdentifier())
      throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
          ": Invalid subroutine call with subroutine name \"" + begin->val() + "\"");
    const Token &name = *begin;
    if (begin + 1 == end)
      throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
          ": Incomplete subroutine call without parameter open symbol \"(\"");
    if (!(begin + 1)->isSymbol('('))
      throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
          ": Invalid subroutine call with parameter open symbol \"" + (begin + 1)->val() + "\"");
    const Token &parameterOpen = *(begin + 1);

    std::vector<Token>::iterator it = begin + 2;
    std::vector< std::unique_ptr<Expression> > expressionList;
    while (it < end && !it->isSymbol(')')) {
      Parser::ExpressionReturnType ret = parseExpression(it, end);
      expressionList.push_back(std::move(ret.parsedExpression));
      it = ret.endIterator;
      if (it < end) {
        if (it->isSymbol(')')) break;
        if (!it->isSymbol(','))
          throw std::runtime_error("Line " + std::to_string(it->lineNumber()) +
              ": Invalid subroutine call expression list with symbol \"" + it->val() + "\"");
      }
      it = ret.endIterator + 1;
    }
    if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
        ": Incomplete subroutine call without close symbol \")\"");
    const Token &parameterClose = *it;

    Parser::SubroutineCallReturnType ret {
      std::make_unique<DirectSubroutineCall>(
          name, parameterOpen, expressionList, parameterClose),
      it + 1
    };
    return ret;
  } else if (begin + 1 != end && (begin + 1)->isSymbol('.')) {
    if (!begin->isIdentifier())
      throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
          ": Invalid subroutine call with class name \"" + begin->val() + "\"");
    const Token &className = *begin;
    const Token &dotSymbol = *(begin + 1);
    if (begin + 2 == end)
      throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
          ": Incomplete subroutine call without subroutine name");
    if (!(begin + 2)->isIdentifier())
      throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
          ": Invalid subroutine call with subroutine name \"" + (begin + 2)->val() + "\"");
    const Token &subroutineName = *(begin + 2);
    if (begin + 3 == end)
      throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
          ": Incomplete subroutine call without open symbol \"(\"");
    if (!(begin + 3)->isSymbol('('))
      throw std::runtime_error("Line " + std::to_string((begin + 3)->lineNumber()) +
          ": Invalid subroutine call with open symbol \"" + (begin + 3)->val() + "\"");
    const Token &parameterOpen = *(begin + 3);

    std::vector<Token>::iterator it = begin + 4;
    std::vector< std::unique_ptr<Expression> > expressionList;
    while (it != end && !it->isSymbol(')')) {
      Parser::ExpressionReturnType ret = parseExpression(it, end);
      expressionList.push_back(std::move(ret.parsedExpression));
      it = ret.endIterator;
      if (it < end) {
        if (it->isSymbol(')')) break;
        if (!it->isSymbol(','))
          throw std::runtime_error("Line " + std::to_string(it->lineNumber()) +
              ": Invalid subroutine call expression list with symbol \"" + it->val() + "\"");
      }
      it = ret.endIterator + 1;
    }
    if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
        ": Incomplete subroutine call without close symbol \")\"");
    const Token &parameterClose = *it;

    Parser::SubroutineCallReturnType ret {
      std::make_unique<IndirectSubroutineCall>(
          className, dotSymbol, subroutineName, parameterOpen, expressionList, parameterClose),
      it + 1
    };
    return ret;
  } else {
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Invalid subroutine call \"" + begin->val() + "\"");
  }
}

Parser::TermReturnType Parser::parseTerm(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  if (begin == end) throw std::runtime_error("Incomplete term");

  Parser::TermReturnType ret;
  if (begin->isIdentifier() &&
      (begin + 1 != end && (begin + 1)->isSymbol('['))) {
    // Array element term
    const Token &name = *begin;
    const Token &idxOpen = *(begin + 1);

    Parser::ExpressionReturnType expRet = parseExpression(begin + 2, end);
    const std::unique_ptr<Expression> &expression = expRet.parsedExpression;

    std::vector<Token>::iterator it = expRet.endIterator;
    if (it == end || !it->isSymbol(']'))
      throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
          ": Invalid array element term with close symbol \"" + it->val() + "\"");
    const Token &idxClose = *it;

    ret.parsedTerm = std::make_unique<ArrayElementTerm>(
        name, idxOpen, expression, idxClose);
    ret.endIterator = it + 1;
  } else if (begin->isIdentifier() &&
      (begin + 1 != end && ((begin + 1)->isSymbol('(') || (begin + 1)->isSymbol('.')))) {
    // Subroutine call term
    Parser::SubroutineCallReturnType subRet = parseSubroutineCall(begin, end);

    ret.parsedTerm = std::make_unique<SubroutineCallTerm>(subRet.parsedSubroutineCall);
    ret.endIterator = subRet.endIterator;
  } else if (begin->isSymbol('(')) {
    // Bracket expression term
    const Token &expressionOpen = *begin;
    Parser::ExpressionReturnType expRet = parseExpression(begin + 1, end);
    if (expRet.endIterator == end)
      throw std::runtime_error("Line " + std::to_string((expRet.endIterator - 1)->lineNumber()) +
          ": Invalid expression term without close symbol \")\"");
    if (!expRet.endIterator->isSymbol(')'))
      throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
          ": Invalid expression term with close symbol \"" + expRet.endIterator->val() + "\"");
    const Token &expressionClose = *expRet.endIterator;

    ret.parsedTerm = std::make_unique<BracketedExpressionTerm>(
        expressionOpen, std::move(expRet.parsedExpression), expressionClose);
    ret.endIterator = expRet.endIterator + 1;
  } else if (begin->isUnaryOperator()) {
    // Unary operator term
    const Token &unaryOp = *begin;
    Parser::TermReturnType termRet = parseTerm(begin + 1, end);

    ret.parsedTerm = std::make_unique<UnaryOpTerm>(unaryOp, termRet.parsedTerm);
    ret.endIterator = termRet.endIterator;
  } else if (begin->isIntegerConstant() || begin->isStringConstant() ||
      begin->isKeyword() || begin->isIdentifier()) {
    // Single term
    ret.parsedTerm = std::make_unique<SingleTerm>(*begin);
    ret.endIterator = begin + 1;
  } else {
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Invalid expression term starting with \"" + begin->val() + "\"");
  }
  return ret;
}

Parser::ExpressionReturnType Parser::parseExpression(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  if (begin == end) throw std::runtime_error("Incomplete expression");

  Parser::TermReturnType termRet1 = parseTerm(begin, end);
  while (termRet1.endIterator != end && termRet1.endIterator->isBinaryOperator()) {
    const Token &op = *termRet1.endIterator;
    Parser::TermReturnType termRet2 = parseTerm(termRet1.endIterator + 1, end);

    if (termRet2.endIterator == end || !termRet2.endIterator->isBinaryOperator()) {
      Parser::ExpressionReturnType ret {
        std::make_unique<BinaryExpression>(
            termRet1.parsedTerm, op, termRet2.parsedTerm),
        termRet2.endIterator
      };
      return ret;
    }

    termRet1.parsedTerm = std::make_unique<ExpressionTerm>(
        std::make_unique<BinaryExpression>(termRet1.parsedTerm, op, termRet2.parsedTerm));
    termRet1.endIterator = termRet2.endIterator;
  }

  Parser::ExpressionReturnType ret {
    std::make_unique<UnaryExpression>(termRet1.parsedTerm),
    termRet1.endIterator
  };
  return ret;
}

Parser::ReturnType Parser::parseStatement(
    const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;
  Parser::ReturnType ret;
  if (keyword.isKeyword("let")) {
    ret = parseLetNode(begin, end);
  } else if (keyword.isKeyword("if")) {
    ret = parseIfNode(begin, end);
  } else if (keyword.isKeyword("while")) {
    ret = parseWhileNode(begin, end);
  } else if (keyword.isKeyword("do")) {
    ret = parseDoNode(begin, end);
  } else if (keyword.isKeyword("return")) {
    ret = parseReturnNode(begin, end);
  } else {
    throw std::runtime_error(
        "Line " + std::to_string(keyword.lineNumber()) +
        ": Expecting a statement, got \"" + keyword.val() + "\" instead");
  }
  return ret;
}

Parser::ReturnType Parser::parseClassNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;
  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Incomplete class declaration without name");
  if (!(begin + 1)->isIdentifier())
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Invalid class declaration with name \"" + (begin + 1)->val() + "\"");
  const Token &name = *(begin + 1);
  if (begin + 2 == end)
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Incomplete class declaration without open symbol \"{\"");
  if (!(begin + 2)->isSymbol('{'))
    throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
        ": Invalid class declaration with open symbol \"" + (begin + 2)->val() + "\"");
  const Token &open = *(begin + 2);

  std::vector<Token>::iterator it = begin + 3;
  std::vector< std::unique_ptr<Node> > nodes;
  bool expectingSubroutine = false;
  while (it != end && !it->isSymbol('}')) {
    const Token &keyword = *it;
    Parser::ReturnType ret;
    if (keyword.isKeyword("static") || keyword.isKeyword("field")) {
      if (expectingSubroutine) {
        throw std::runtime_error(
            "Line " + std::to_string(keyword.lineNumber()) +
            ": Expecting subroutine declaration, got class variable " +
            " declaration instead");
      }
      ret = parseDeclarationNode(it, end);
    } else if (keyword.isKeyword("constructor") ||
               keyword.isKeyword("function") ||
               keyword.isKeyword("method")) {
      ret = parseSubroutineNode(it, end);
      expectingSubroutine = true;
    } else {
      throw std::runtime_error(
          "Line " + std::to_string(keyword.lineNumber()) +
          ": Expecting subroutine declaration" + 
          (expectingSubroutine ? "" : " or class variable declaration") +
          ", got token \"" + keyword.val() + "\" instead"
      );
    }
    nodes.push_back(std::move(ret.parsedNode));
    it = ret.endIterator;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Incomplete class declaration without close symbol \"}\"");
  const Token &close = *it;

  Parser::ReturnType ret {
    std::make_unique<ClassNode>(keyword, name, open, close),
    it + 1
  };
  ret.parsedNode->addAll(nodes);
  return ret;
}

Parser::ReturnType Parser::parseSubroutineNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;
  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Incomplete subroutine declaration without return type");
  if (!(begin + 1)->isKeyword() && !(begin + 1)->isIdentifier())
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Invalid subroutine declaration with return type \"" + (begin + 1)->val() + "\"");
  const Token &returnType = *(begin + 1);
  if (begin + 2 == end)
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Incomplete subroutine declaration without name");
  if (!(begin + 2)->isIdentifier())
    throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
        ": Invalid subroutine declaration with name \"" + (begin + 2)->val() + "\"");
  const Token &name = *(begin + 2);
  if (begin + 3 == end)
    throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
        ": Incomplete subroutine declaration without parameter open symbol \"(\"");
  if (!(begin + 3)->isSymbol('('))
    throw std::runtime_error("Line " + std::to_string((begin + 3)->lineNumber()) +
        ": Invalid class declaration with parameter open symbol \"" + (begin + 3)->val() + "\"");
  const Token &parameterOpen = *(begin + 3);

  std::vector<Token>::iterator it = begin + 4;
  std::vector<Token> parameters;
  int state = 0;
  while (it != end && !it->isSymbol(')')) {
    const Token &token = *it;
    switch (state) {
      case 0:
      case 1:
        if (!token.isIdentifier() && !token.isKeyword("int") &&
            !token.isKeyword("char") && !token.isKeyword("boolean")) {
          throw std::runtime_error(
            "Line " + std::to_string(token.lineNumber()) +
            ": Expecting an identifier, \"int\", \"char\" or \"boolean\", " +
            "got \"" + token.val() + "\" instead");
        }
        break;
      case 2:
        if (!token.isSymbol(',')) {
          throw std::runtime_error(
            "Line " + std::to_string(token.lineNumber()) +
            ": Expecting a \",\", got \"" + token.val() + "\" instead");
        }
        break;
    }
    parameters.push_back(token);
    ++it;
    state = (state + 1) % 3;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Incomplete subroutine declaration without parameter close symbol \")\"");
  const Token &parameterClose = *it;

  if (it + 1 == end)
    throw std::runtime_error("Line " + std::to_string(it->lineNumber()) +
        ": Incomplete subroutine declaration without open symbol \"{\"");
  if (!(it + 1)->isSymbol('{'))
    throw std::runtime_error("Line " + std::to_string((it + 1)->lineNumber()) +
        ": Invalid subroutine declaration with open symbol \"" + (it + 1)->val() + "\"");
  const Token &open = *(it + 1);

  it += 2;
  std::vector< std::unique_ptr<Node> > nodes;
  bool expectingStatement = false;
  while (it != end && !it->isSymbol('}')) {
    const Token &token = *it;
    Parser::ReturnType ret;
    if (token.isKeyword("var")) {
      if (expectingStatement) {
        throw std::runtime_error("Line " + std::to_string(it->lineNumber()) +
            ": Expecting a statement, got \"" + token.val() + "\" instead");
      }
      ret = parseDeclarationNode(it, end);
    } else if (token.isKeyword("let") || token.isKeyword("if") ||
               token.isKeyword("while") || token.isKeyword("do") ||
               token.isKeyword("return")) {
      expectingStatement = true;
      ret = parseStatement(it, end);
    } else {
        throw std::runtime_error("Line " + std::to_string(it->lineNumber()) +
            ": Expecting a statement" +
            (expectingStatement ? "" : " or variable declaration") +
            ", got \"" + token.val() + "\" instead");
    }
    nodes.push_back(std::move(ret.parsedNode));
    it = ret.endIterator;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Incomplete subroutine declaration without close symbol \"}\"");
  const Token &close = *it;

  Parser::ReturnType ret {
    std::make_unique<SubroutineNode>(keyword, returnType, name,
          parameterOpen, parameters, parameterClose, open, close),
    it + 1
  };
  ret.parsedNode->addAll(nodes);
  return ret;
}

Parser::ReturnType Parser::parseDeclarationNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;

  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
      ": Incomplete variable declaration without type");
  if (!(begin + 1)->isIdentifier() && !(begin + 1)->isKeyword())
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
      ": Invalid variable declaration with type \"" + (begin + 1)->val() + "\"");
  const Token &type = *(begin + 1);

  std::vector<Token>::iterator it = begin + 2;
  std::vector<Token> variableList;
  bool expectingComma = false;
  while (it != end && !it->isSymbol(';')) {
    const Token &token = *it;
    if (expectingComma) {
      if (!token.isSymbol(',')) {
        throw std::runtime_error(
            "Line " + std::to_string(token.lineNumber()) +
            ": Expecting \",\", got \"" + token.val() + "\" instead");
      }
    } else {
      if (!token.isIdentifier()) {
        throw std::runtime_error(
            "Line " + std::to_string(token.lineNumber()) +
            ": Expecting an identifier, got \"" + token.val() +
            "\" instead");
      }
      variableList.push_back(token);
    }
    ++it;
    expectingComma = !expectingComma;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Invalid variable declaration without close symbol \";\"");
  const Token &close = *it;

  Parser::ReturnType ret {
    std::make_unique<DeclarationNode>(keyword, type, variableList, close),
    it + 1
  };
  return ret;
}

Parser::ReturnType Parser::parseLetNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;
  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Incomplete let statement without variable name");
  if (!(begin + 1)->isIdentifier())
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Invalid let statement with variable name \"" + (begin + 1)->val() + "\"");
  const Token &name = *(begin + 1);
  if (begin + 2 != end && (begin + 2)->isSymbol('[')) {
    // Let statement with array access
    const Token &idxOpen = *(begin + 2);
    Parser::ExpressionReturnType expRet1 = parseExpression(begin + 3, end);
    if (expRet1.endIterator == end)
      throw std::runtime_error("Line " + std::to_string((expRet1.endIterator - 1)->lineNumber()) +
          ": Incomplete let statement without index close symbol \"]\"");
    if (!expRet1.endIterator->isSymbol(']'))
      throw std::runtime_error("Line " + std::to_string(expRet1.endIterator->lineNumber()) +
          "Invalid let statement with index close symbol \"" + expRet1.endIterator->val() + "\"");
    const Token &idxClose = *expRet1.endIterator;
    if (expRet1.endIterator + 1 == end)
      throw std::runtime_error("Line " + std::to_string(expRet1.endIterator->lineNumber()) +
          ": Incomplete let statement without equal symbol \"=\"");
    if (!(expRet1.endIterator + 1)->isSymbol('='))
      throw std::runtime_error("Line " + std::to_string((expRet1.endIterator + 1)->lineNumber()) +
          ": Invalid let statement with equal symbol \"" + (expRet1.endIterator + 1)->val() + "\"");
    const Token &eqSymbol = *(expRet1.endIterator + 1);
    Parser::ExpressionReturnType expRet2 = parseExpression(expRet1.endIterator + 2, end);
    if (expRet2.endIterator == end)
      throw std::runtime_error("Line " + std::to_string((expRet2.endIterator - 1)->lineNumber()) +
          ": Incomplete let statement without close symbol \";\"");
    if (!expRet2.endIterator->isSymbol(';'))
      throw std::runtime_error("Line " + std::to_string(expRet2.endIterator->lineNumber()) +
          ": Invalid let statement with close symbol \"" + expRet2.endIterator->val() + "\"");
    const Token &close = *expRet2.endIterator;

    Parser::ReturnType ret {
      std::make_unique<ArrayLetNode>(
          keyword, name, idxOpen, expRet1.parsedExpression, idxClose,
            eqSymbol, expRet2.parsedExpression, close),
      expRet2.endIterator + 1
    };
    return ret;
  } else {
    // Let statement without array access
    if (begin + 2 == end)
      throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
          ": Incomplete let statement without equal symbol \"=\"");
    if (!(begin + 2)->isSymbol('='))
      throw std::runtime_error("Line " + std::to_string((begin + 2)->lineNumber()) +
          ": Invalid let statement with equal symbol \"" + (begin + 2)->val() + "\"");
    const Token &eqSymbol = *(begin + 2);
    Parser::ExpressionReturnType expRet = parseExpression(begin + 3, end);
    if (expRet.endIterator == end)
      throw std::runtime_error("Line " + std::to_string((expRet.endIterator - 1)->lineNumber()) +
          ": Incomplete let statement without close symbol \";\"");
    if (!expRet.endIterator->isSymbol(';'))
      throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
          ": Invalid let statement with close symbol \"" + expRet.endIterator->val() + "\"");
    const Token &close = *expRet.endIterator;

    Parser::ReturnType ret {
      std::make_unique<VariableLetNode>(
          keyword, name, eqSymbol, expRet.parsedExpression, close),
      expRet.endIterator + 1
    };
    return ret;
  }
}

Parser::ReturnType Parser::parseDoNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;

  Parser::SubroutineCallReturnType subRet = parseSubroutineCall(begin + 1, end);
  if (subRet.endIterator == end)
    throw std::runtime_error("Line " + std::to_string((subRet.endIterator - 1)->lineNumber()) +
        ": Incomplete do statement without close symbol \";\"");
  if (!subRet.endIterator->isSymbol(';'))
    throw std::runtime_error("Line " + std::to_string(subRet.endIterator->lineNumber()) +
        ": Invalid do statement with close symbol \"" + subRet.endIterator->val() + "\"");
  const Token &close = *subRet.endIterator;

  Parser::ReturnType ret {
    std::make_unique<DoNode>(keyword, subRet.parsedSubroutineCall, close),
    subRet.endIterator + 1
  };
  return ret;
}

Parser::ReturnType Parser::parseIfNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &ifKeyword = *begin;

  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Incomplete if statement without expression open symbol \"(\"");
  if (!(begin + 1)->isSymbol('('))
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Invalid if statement with expression open symbol \"" + (begin + 1)->val() + "\"");
  const Token &expressionOpen = *(begin + 1);
  Parser::ExpressionReturnType expRet = parseExpression(begin + 2, end);
  if (expRet.endIterator == end)
    throw std::runtime_error("Line " + std::to_string((expRet.endIterator - 1)->lineNumber()) +
        ": Incomplete if statement without expression close symbol\")\"");
  if (!expRet.endIterator->isSymbol(')'))
    throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
        ": Invalid if statement with expression close symbol \"" + expRet.endIterator->val() + "\"");
  const Token &expressionClose = *expRet.endIterator;
  if (expRet.endIterator + 1 == end)
    throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
        ": Incomplete if statement without if open symbol\"{\"");
  if (!(expRet.endIterator + 1)->isSymbol('{'))
    throw std::runtime_error("Line " + std::to_string((expRet.endIterator + 1)->lineNumber()) +
        ": Invalid if statement with if open symbol \"" + (expRet.endIterator + 1)->val() + "\"");
  const Token &ifOpen = *(expRet.endIterator + 1);

  std::vector< std::unique_ptr<Node> > nodes;
  std::vector<Token>::iterator it = expRet.endIterator + 2;
  while (it != end && !it->isSymbol('}')) {
    Parser::ReturnType ret = parseStatement(it, end);
    nodes.push_back(std::move(ret.parsedNode));
    it = ret.endIterator;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Incomplete if statement without if close symbol \"}\"");
  const Token &ifClose = *it;

  if (it + 1 != end && (it + 1)->isKeyword("else")) {
    std::size_t elseBegin = nodes.size();
    const Token &elseKeyword = *(it + 1);
    if (it + 2 == end)
      throw std::runtime_error("Line " + std::to_string((it + 1)->lineNumber()) +
          ": Incomplete if statement without else open symbol\"{\"");
    if (!(it + 2)->isSymbol('{'))
      throw std::runtime_error("Line " + std::to_string((it + 2)->lineNumber()) +
          ": Invalid if statement with if open symbol \"" + (it + 2)->val() + "\"");
    const Token &elseOpen = *(it + 2);

    it += 3;
    while (it != end && !it->isSymbol('}')) {
      Parser::ReturnType ret = parseStatement(it, end);
      nodes.push_back(std::move(ret.parsedNode));
      it = ret.endIterator;
    }
    if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
        ": Incomplete if statement without else close symbol \"}\"");
    const Token &elseClose = *it;

    Parser::ReturnType ret {
      std::make_unique<IfElseIfNode>(
          ifKeyword, expressionOpen, expRet.parsedExpression, expressionClose,
            ifOpen, ifClose, elseBegin, elseKeyword, elseOpen, elseClose),
      it + 1
    };
    ret.parsedNode->addAll(nodes);
    return ret;
  } else {
    Parser::ReturnType ret {
      std::make_unique<SingleIfNode>(
          ifKeyword, expressionOpen, expRet.parsedExpression, expressionClose,
            ifOpen, ifClose),
      it + 1
    };
    ret.parsedNode->addAll(nodes);
    return ret;
  }
}

Parser::ReturnType Parser::parseWhileNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;

  if (begin + 1 == end)
    throw std::runtime_error("Line " + std::to_string(begin->lineNumber()) +
        ": Incomplete while statement without expression open symbol \"(\"");
  if (!(begin + 1)->isSymbol('('))
    throw std::runtime_error("Line " + std::to_string((begin + 1)->lineNumber()) +
        ": Invalid while statement with expression open symbol \"" + (begin + 1)->val() + "\"");
  const Token &expressionOpen = *(begin + 1);
  Parser::ExpressionReturnType expRet = parseExpression(begin + 2, end);
  if (expRet.endIterator == end)
    throw std::runtime_error("Line " + std::to_string((expRet.endIterator - 1)->lineNumber()) +
        ": Incomplete while statement without expression close symbol\")\"");
  if (!expRet.endIterator->isSymbol(')'))
    throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
        ": Invalid while statement with expression close symbol \"" + expRet.endIterator->val() + "\"");
  const Token &expressionClose = *expRet.endIterator;
  if (expRet.endIterator + 1 == end)
    throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
        ": Incomplete while statement without open symbol\"{\"");
  if (!(expRet.endIterator + 1)->isSymbol('{'))
    throw std::runtime_error("Line " + std::to_string((expRet.endIterator + 1)->lineNumber()) +
        ": Invalid while statement with open symbol \"" + (expRet.endIterator + 1)->val() + "\"");
  const Token &open = *(expRet.endIterator + 1);

  std::vector< std::unique_ptr<Node> > nodes;
  std::vector<Token>::iterator it = expRet.endIterator + 2;
  while (it != end && !it->isSymbol('}')) {
    Parser::ReturnType ret = parseStatement(it, end);
    nodes.push_back(std::move(ret.parsedNode));
    it = ret.endIterator;
  }
  if (it == end) throw std::runtime_error("Line " + std::to_string((it - 1)->lineNumber()) +
      ": Incomplete while statement without close symbol \"}\"");
  const Token &close = *it;

  Parser::ReturnType ret {
    std::make_unique<WhileNode>(
        keyword, expressionOpen, expRet.parsedExpression, expressionClose, open, close),
    it + 1
  };
  ret.parsedNode->addAll(nodes);
  return ret;
}

Parser::ReturnType Parser::parseReturnNode(const std::vector<Token>::iterator &begin,
    const std::vector<Token>::iterator &end) const {
  const Token &keyword = *begin;

  if (begin + 1 != end && (begin + 1)->isSymbol(';')) {
    const Token &close = *(begin + 1);

    Parser::ReturnType ret {
      std::make_unique<ExpressionlessReturnNode>(keyword, close),
      begin + 2
    };
    return ret;
  } else {
    Parser::ExpressionReturnType expRet = parseExpression(begin + 1, end);
    if (expRet.endIterator == end)
      throw std::runtime_error("Line " + std::to_string(((expRet.endIterator - 1)->lineNumber())) +
          ": Incomplete return statement without close symbol \";\"");
    if (!expRet.endIterator->isSymbol(';'))
      throw std::runtime_error("Line " + std::to_string(expRet.endIterator->lineNumber()) +
          ": Invalid return statement with close symbol \"" + expRet.endIterator->val() + "\"");
    const Token &close = *expRet.endIterator;

    Parser::ReturnType ret {
      std::make_unique<ExpressionReturnNode>(
          keyword, expRet.parsedExpression, close),
      expRet.endIterator + 1
    };
    return ret;
  }
}

