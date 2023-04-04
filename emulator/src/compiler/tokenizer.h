#pragma once

#include <vector>
#include <fstream>
#include <string>
#include <unordered_set>

#define IS_SINGLE_LINE_COMMENT_OPEN(X)  (*(X) == '/' && *(X + 1) == '/')
#define IS_SINGLE_LINE_COMMENT_CLOSE(X) (*(X) == '\n' || *(X) == '\r')
#define IS_MULTI_LINE_COMMENT_OPEN(X)   (*(X) == '/' && *(X + 1) == '*')
#define IS_MULTI_LINE_COMMENT_CLOSE(X)  (*(X) == '*' && *(X + 1) == '/')

#define IS_SKIPPABLE(X)                 (*(X) == ' ' || *(X) == '\t' || *(X) == '\n' || *(X) == '\r')
#define IS_NEWLINE(X)                   (*(X) == '\n')
#define IS_NULL_BYTE(X)                 (*(X) == '\0')

#define IS_DIGIT(X)                     (*(X) >= '0' && *(X) <= '9')
#define IS_LETTER(X)                    ((*(X) >= 'A' && *(X) <= 'Z') || (*(X) >= 'a' && *(X) <= 'z'))

#define IS_STRING_CONSTANT_OPEN(X)      (*(X) == '\"')
#define IS_STRING_CONSTANT_CLOSE(X)     (*(X) == '\"')

#define IS_IDENTIFIER_OPEN(X)           (IS_LETTER(X) || *(X) == '_')
#define IS_IDENTIFIER_CONTENT(X)        (IS_DIGIT(X) || IS_LETTER(X) || *(X) == '_')

class Token {
public:
  enum class Type {
    INTEGER_CONSTANT,
    STRING_CONSTANT,
    IDENTIFIER,
    KEYWORD,
    SYMBOL,
    ARGUMENT
  };

  Token() = default;
  Token(const Token&) = default;
  Token(Token&&) = default;
  Token(std::size_t lineNumber, const std::string &val, Type type);

  Token &operator=(const Token &) = default;

  bool isIntegerConstant() const;
  bool isStringConstant() const;
  bool isIdentifier() const;
  bool isKeyword(const std::string &keyword = "") const;
  bool isSymbol(char symbol = '\0') const;
  bool isUnaryOperator() const;
  bool isBinaryOperator() const;

  std::size_t lineNumber() const;
  std::string val() const;

  void toXML(std::fstream &outputFile, const std::string &indentation = "") const;
  void print() const;

private:
  std::size_t lineNumber_;
  std::string val_;
  Type type_;
};

class Tokens {
public:
  void add(const Token &token);
  void toXML(std::fstream &outputFile) const;
  void print() const;

  std::vector<Token>::iterator begin();
  std::vector<Token>::const_iterator begin() const;
  std::vector<Token>::iterator end();
  std::vector<Token>::const_iterator end() const;

private:
  std::vector<Token> tokens_;
};

class Tokenizer {
public:
  Tokenizer(const std::string &inputFileName);
  Tokenizer(const std::string &inputFileName, const std::string &input);

  static void init();

  Tokens tokenize() const;
  void toXML(const std::string &outputFileName) const;

  static bool IS_SYMBOL(const std::string::const_iterator &it);
  static bool IS_KEYWORD(const std::string &word);

private:
  static bool initialized_;
  static std::unordered_set<char> symbols_;
  static std::unordered_set<std::string> keywords_;

  std::string inputFileName_;
  std::string input_;
};

