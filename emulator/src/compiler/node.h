#pragma once

#include "tokenizer.h"
#include "vmcommand.h"

#include <vector>
#include <fstream>
#include <memory>
#include <unordered_map>

class Node {
public:
  enum class Type {
    CLASS,
    SUBROUTINE,
    DECLARATION,
    LET,
    DO,
    IF,
    WHILE,
    RETURN,
    ANY
  };

  struct TableElement {
    TableElement(const Token &type, const Token &kind, std::size_t idx);
    TableElement() = default;
    TableElement(const TableElement &) = default;
    Token type;
    Token kind;
    std::size_t idx;
  };

  struct SymbolTable {
    std::string className;
    std::unordered_map<std::string, TableElement> staticTable;
    std::unordered_map<std::string, TableElement> fieldTable;
    std::unordered_map<std::string, TableElement> argumentTable;
    std::unordered_map<std::string, TableElement> localTable;
  };

  Node(Type type);
  virtual ~Node() = default;

  bool isDeclaration() const;
  bool isStatement(Type type = Type::ANY) const;

  virtual bool hasReturnStatement() const;

  void add(const std::unique_ptr<Node> &node);
  void addAll(const std::vector< std::unique_ptr<Node> > &nodes);

  virtual std::unique_ptr<Node> clone() const = 0;

  virtual void toXML(std::fstream &outputFile, const std::string &indentation = "") const = 0;
  virtual SymbolTable toVMCommands(VMCommands &vmCommands, SymbolTable symbolTable) const = 0;

protected:
  Type type_;
  std::vector< std::unique_ptr<Node> > children_;
};

class Nodes {
public:
  void add(const std::unique_ptr<Node> &node);
  void toXML(std::fstream &outputFile, const std::string &indentation = "") const;
  VMCommands toVMCommands() const;

  std::vector< std::unique_ptr<Node> >::iterator begin();
  std::vector< std::unique_ptr<Node> >::const_iterator begin() const;
  std::vector< std::unique_ptr<Node> >::iterator end();
  std::vector< std::unique_ptr<Node> >::const_iterator end() const;
  std::size_t size() const;

private:
  std::vector< std::unique_ptr<Node> > nodes_;
};

