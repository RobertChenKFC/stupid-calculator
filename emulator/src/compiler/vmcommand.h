#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <fstream>
#include <unordered_map>

#define TO_SEGMENT_STRING(X) SEGMENT_STRINGS[static_cast<int>(X)]

using Word = std::int16_t;

class VMCommand {
public:
  enum class Operation {
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT,
    PUSH,
    POP,
    LABEL,
    GOTO,
    IF_GOTO,
    FUNCTION,
    CALL,
    RETURN
  };

  enum class Segment {
    STATIC,
    THIS,
    LOCAL,
    ARGUMENT,
    THAT,
    CONSTANT,
    POINTER,
    TEMP
  };

  static constexpr const char *SEGMENT_STRINGS[] = {
    "static",
    "this",
    "local",
    "argument",
    "that",
    "constant",
    "pointer",
    "temp"
  };

  VMCommand(Operation op);
  VMCommand(Operation op, Segment segment, Word offset);
  VMCommand(Operation op, const std::string &label);
  VMCommand(Operation op, const std::string &func, Word n);
  VMCommand() = default;
  VMCommand(const VMCommand &) = default;
  VMCommand(VMCommand &&) = default;

  VMCommand& operator=(const VMCommand &) = default;
  VMCommand& operator=(VMCommand &&) = default;

  void toVMCode(std::iostream &outputFile) const;

  void print() const;

  friend class VMCommands;

private:
  Operation op_;
  Segment segment_;
  std::string str_;
  Word label_;
  Word n_;
};

class VMCommands {
public:
  struct PosInfo {
    std::size_t staticIdx;
    std::size_t funcIdx;
  };

  VMCommands();
  VMCommands(const std::string& inputFileName);

  static void init();

  void add(const VMCommand &vmCommand);
  void add(VMCommand &&vmCommand);
  void add(const VMCommands &vmCommands);
  void add(VMCommands &&vmCommands);

  std::vector<VMCommand>::iterator begin();
  std::vector<VMCommand>::const_iterator begin() const;
  std::vector<VMCommand>::iterator end();
  std::vector<VMCommand>::const_iterator end() const;
  VMCommand& operator[](std::size_t idx);
  const VMCommand& operator[](std::size_t idx) const;
  std::size_t size() const;

  void setMemoryPtr(Word *ptr);
  void setKey(Word key);
  void initExecution();
  bool execute(std::size_t steps);

  void reset();
  void clear();

  void toVMCode(std::iostream &outputFile) const;
  std::string toVMCodeString() const;

private:
  void push_(VMCommand::Segment segment, Word offset) const;
  void pop_(VMCommand::Segment segment, Word offset) const;

  static bool initialized_;
  static std::unordered_map<std::string, VMCommand::Operation> operations_;
  static std::unordered_map<std::string, VMCommand::Segment> segments_;

  Word *memory_;
  // TODO: change segment offset
  static constexpr Word SP = 0, LCL = 1, ARG = 2, THIS = 3, THAT = 4, TEMP = 5;
  static constexpr Word STATIC = 16, STACK = 256, HEAP = 2048, KBD = 24576;

  bool initializedExecution_;
  static Word numStatics_;
  std::size_t startPos_;
  std::size_t curPos_;
  std::vector<VMCommand> vmCommands_;
};

