#include "tokenizer.h"

#include <iostream>
#include <exception>

Token::Token(std::size_t lineNumber, const std::string &val, Type type) :
  lineNumber_(lineNumber), val_(val), type_(type)
{

}

void Token::toXML(std::fstream &outputFile, const std::string &indentation) const {
  outputFile << indentation;
  switch (type_) {
    case Type::INTEGER_CONSTANT:
      outputFile << "<integerConstant> " << val_ << " </integerConstant>" << std::endl;
      break;
    case Type::STRING_CONSTANT:
      outputFile << "<stringConstant> " << val_ << " </stringConstant>" << std::endl;
      break;
    case Type::IDENTIFIER:
      outputFile << "<identifier> " << val_ << " </identifier>" << std::endl;
      break;
    case Type::KEYWORD:
      outputFile << "<keyword> " << val_ << " </keyword>" << std::endl;
      break;
    case Type::SYMBOL:
      outputFile << "<symbol> ";
      switch (val_[0]) {
        case '<':
          outputFile << "&lt;";
          break;
        case '>':
          outputFile << "&gt;";
          break;
        case '&':
          outputFile << "&amp;";
          break;
        default:
          outputFile << val_[0];
      }
      outputFile << " </symbol>" << std::endl;
      break;
    default:
      break;
  }
}

void Token::print() const {
  std::cout << "line: " << lineNumber_ << ", val: " << val_ << ", type: ";
  switch (type_) {
    case Type::INTEGER_CONSTANT:
      std::cout << "int" << std::endl;
      break;
    case Type::STRING_CONSTANT:
      std::cout << "string" << std::endl;
      break;
    case Type::IDENTIFIER:
      std::cout << "identifier" << std::endl;
      break;
    case Type::KEYWORD:
      std::cout << "keyword" << std::endl;
      break;
    case Type::SYMBOL:
      std::cout << "symbol" << std::endl;
      break;
    case Type::ARGUMENT:
      std::cout << "argument" << std::endl;
      break;
    default:
      std::cout << "Invalid token " << static_cast<int>(type_) << std::endl;
  }
}

bool Token::isIntegerConstant() const {
  return type_ == Type::INTEGER_CONSTANT;
}

bool Token::isStringConstant() const {
  return type_ == Type::STRING_CONSTANT;
}

bool Token::isIdentifier() const {
  return type_ == Type::IDENTIFIER;
}

bool Token::isKeyword(const std::string &keyword) const {
  if (keyword.empty()) return type_ == Type::KEYWORD;
  return type_ == Type::KEYWORD && val_ == keyword;
}

bool Token::isSymbol(char symbol) const {
  if (symbol == '\0') return type_ == Type::SYMBOL;
  return type_ == Type::SYMBOL && val_[0] == symbol;
}

bool Token::isUnaryOperator() const {
  return (type_ == Type::SYMBOL) &&
    (val_[0] == '-' || val_[0] == '~');
}

bool Token::isBinaryOperator() const {
  return (type_ == Type::SYMBOL) &&
    (val_[0] == '+' || val_[0] == '-' || val_[0] == '*' || val_[0] == '/' ||
     val_[0] == '&' || val_[0] == '|' || val_[0] == '<' || val_[0] == '>' ||
     val_[0] == '=');
}

std::size_t Token::lineNumber() const {
  return lineNumber_;
}

std::string Token::val() const {
  return val_;
}

void Tokens::add(const Token &token) {
  tokens_.push_back(token);
}

void Tokens::toXML(std::fstream &outputFile) const {
  outputFile << "<tokens>" << std::endl;
  for (const Token &token : tokens_) token.toXML(outputFile);
  outputFile << "</tokens>" << std::endl;
}

void Tokens::print() const {
  for (const Token &token : tokens_)
    token.print();
}

std::vector<Token>::iterator Tokens::begin() {
  return tokens_.begin();
}

std::vector<Token>::const_iterator Tokens::begin() const {
  return tokens_.begin();
}

std::vector<Token>::iterator Tokens::end() {
  return tokens_.end();
}

std::vector<Token>::const_iterator Tokens::end() const {
  return tokens_.end();
}

Tokenizer::Tokenizer(const std::string &inputFileName) :
  inputFileName_(inputFileName)
{
  std::ifstream inputFile(inputFileName);
  inputFile.seekg(0, std::ios::end);
  input_.reserve(static_cast<std::size_t>(inputFile.tellg()));
  inputFile.seekg(0, std::ios::beg);
  input_.assign(std::istreambuf_iterator<char>(inputFile),
      std::istreambuf_iterator<char>());
  input_.push_back('\0');
}

Tokenizer::Tokenizer(const std::string &inputFileName, const std::string &input) :
  inputFileName_(inputFileName),
  input_(input)
{

}

bool Tokenizer::initialized_ = false;
std::unordered_set<char> Tokenizer::symbols_;
std::unordered_set<std::string> Tokenizer::keywords_;
void Tokenizer::init() {
  if (initialized_) return;

  symbols_.insert('(');
  symbols_.insert(')');
  symbols_.insert('[');
  symbols_.insert(']');
  symbols_.insert('{');
  symbols_.insert('}');
  symbols_.insert(',');
  symbols_.insert(';');
  symbols_.insert('=');
  symbols_.insert('.');
  symbols_.insert('+');
  symbols_.insert('-');
  symbols_.insert('*');
  symbols_.insert('/');
  symbols_.insert('&');
  symbols_.insert('|');
  symbols_.insert('~');
  symbols_.insert('>');
  symbols_.insert('<');

  keywords_.insert("class");
  keywords_.insert("constructor");
  keywords_.insert("method");
  keywords_.insert("function");
  keywords_.insert("int");
  keywords_.insert("boolean");
  keywords_.insert("char");
  keywords_.insert("void");
  keywords_.insert("var");
  keywords_.insert("static");
  keywords_.insert("field");
  keywords_.insert("let");
  keywords_.insert("do");
  keywords_.insert("if");
  keywords_.insert("else");
  keywords_.insert("while");
  keywords_.insert("return");
  keywords_.insert("true");
  keywords_.insert("false");
  keywords_.insert("null");
  keywords_.insert("this");

  initialized_ = true;
}

bool Tokenizer::IS_SYMBOL(const std::string::const_iterator &it) {
  return symbols_.find(*it) != symbols_.end();
}

bool Tokenizer::IS_KEYWORD(const std::string &it) {
  return keywords_.find(it) != keywords_.end();
}

Tokens Tokenizer::tokenize() const {
  if (!initialized_) throw std::runtime_error("Please call Tokenizer::init before tokenizing!");

  Tokens tokens;
  std::size_t lineNumber = 1;
  for (auto it = input_.begin(), end = input_.end(); it < end && !IS_NULL_BYTE(it);) {
    while (IS_SKIPPABLE(it) && it < end) {
      if (IS_NEWLINE(it)) ++lineNumber;
      ++it;
    }
    if (it >= end || IS_NULL_BYTE(it)) break;
    if (IS_SINGLE_LINE_COMMENT_OPEN(it)) {
      while (!IS_SINGLE_LINE_COMMENT_CLOSE(it) && it < end) ++it;
    } else if (IS_MULTI_LINE_COMMENT_OPEN(it)) {
      while (!IS_MULTI_LINE_COMMENT_CLOSE(it) && it < end) {
        if (IS_NEWLINE(it)) ++lineNumber;
        ++it;
      }
      it += 2;
    } else if (IS_DIGIT(it)){
      std::string integerConstant;
      while (IS_DIGIT(it) && it < end) {
        integerConstant.push_back(*it);
        ++it;
      }
      tokens.add(Token(lineNumber, integerConstant, Token::Type::INTEGER_CONSTANT));
    } else if (IS_STRING_CONSTANT_OPEN(it)) {
      ++it;
      std::string stringConstant;
      while (!IS_STRING_CONSTANT_CLOSE(it) && it < end) {
        stringConstant.push_back(*it);
        ++it;
      }
      tokens.add(Token(lineNumber, stringConstant, Token::Type::STRING_CONSTANT));
      ++it;
    } else if (IS_IDENTIFIER_OPEN(it)) {
      std::string identifier;
      identifier.push_back(*it);
      ++it;
      while (IS_IDENTIFIER_CONTENT(it) && it < end) {
        identifier.push_back(*it);
        ++it;
      }
      tokens.add(Token(lineNumber, identifier,
            IS_KEYWORD(identifier) ? Token::Type::KEYWORD : Token::Type::IDENTIFIER));
    } else if (IS_SYMBOL(it)) {
      std::string symbol(1, *it);
      tokens.add(Token(lineNumber, symbol, Token::Type::SYMBOL));
      ++it;
    } else {
      throw std::runtime_error("Line " + std::to_string(lineNumber) + ": Invalid token \"" + std::string(1, *it) + "\"");
    }
  }
  return tokens;
}

void Tokenizer::toXML(const std::string &outputFileName) const {
  std::fstream outputFile(outputFileName, std::ios::out);

  const Tokens &tokens = tokenize();
  tokens.toXML(outputFile);

  outputFile.close();
}

