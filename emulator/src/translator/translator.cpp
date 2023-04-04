#include "translator.h"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <vector>

enum class Segment {
  CONSTANT,
  LOCAL,
  ARGUMENT,
  THIS,
  THAT,
  POINTER,
  TMP,
  STATIC,
  VALUE,
  FRAME
};

void UnaryOperation(std::iostream &outputFile, char operation) {
  outputFile << "@SP" << std::endl
    << "A=M-1" << std::endl
    << "M=" << operation << "M" << std::endl;
}

void BinaryOperation(std::iostream &outputFile, char operation) {
  outputFile << "@SP" << std::endl
    << "AM=M-1" << std::endl
    << "D=M" << std::endl
    << "A=A-1" << std::endl;
  if (operation == '-')
    outputFile << "M=M-D" << std::endl;
  else
    outputFile << "M=D" << operation << "M" << std::endl;
}

void CompareOperation(
    std::iostream &outputFile, const std::string &operation, Word &labelNum) {
  outputFile << "@SP" << std::endl
    << "AM=M-1" << std::endl
    << "D=M" << std::endl
    << "A=A-1" << std::endl
    << "D=M-D" << std::endl
    << "@TRANSLATOR_CMP_BEGIN" << labelNum << std::endl
    << "D;" << operation << std::endl
    << "D=0" << std::endl
    << "@TRANSLATOR_CMP_END" << labelNum << std::endl
    << "0;JMP" << std::endl
    << "(TRANSLATOR_CMP_BEGIN" << labelNum << ")" << std::endl
    << "D=-1" << std::endl
    << "(TRANSLATOR_CMP_END" << labelNum << ")" << std::endl
    << "@SP" << std::endl
    << "A=M-1" << std::endl
    << "M=D" << std::endl;
  ++labelNum;
}

void Push(std::iostream &outputFile, Segment segment, const std::string &value) {
  std::string segmentAddress;
  switch (segment) {
    case Segment::CONSTANT:
      outputFile << "@" << value << std::endl
        << "D=A" << std::endl
        << "@SP" << std::endl
        << "A=M" << std::endl
        << "M=D" << std::endl
        << "@SP" << std::endl
        << "M=M+1" << std::endl;
      return;
    case Segment::LOCAL:
      segmentAddress = "LCL";
      break;
    case Segment::ARGUMENT:
      segmentAddress = "ARG";
      break;
    case Segment::THIS:
      segmentAddress = "THIS";
      break;
    case Segment::THAT:
      segmentAddress = "THAT";
      break;
    case Segment::TMP:
      outputFile << "@" << 5 + std::stoi(value) << std::endl
        << "D=M" << std::endl
        << "@SP" << std::endl
        << "A=M" << std::endl
        << "M=D" << std::endl
        << "@SP" << std::endl
        << "M=M+1" << std::endl;
      return;
    case Segment::POINTER:
      outputFile << "@" << 3 + std::stoi(value) << std::endl
        << "D=M" << std::endl
        << "@SP" << std::endl
        << "A=M" << std::endl
        << "M=D" << std::endl
        << "@SP" << std::endl
        << "M=M+1" << std::endl;
      return;
    case Segment::STATIC:
      outputFile << "@" << value << std::endl
        << "D=M" << std::endl
        << "@SP" << std::endl
        << "A=M" << std::endl
        << "M=D" << std::endl
        << "@SP" << std::endl
        << "M=M+1" << std::endl;
      return;
    case Segment::VALUE:
      outputFile << "@" << value << std::endl
        << "D=M" << std::endl
        << "@SP" << std::endl
        << "A=M" << std::endl
        << "M=D" << std::endl
        << "@SP" << std::endl
        << "M=M+1" << std::endl;
      return;
    default:
      return;
  }

  outputFile << "@" << segmentAddress << std::endl
    << "D=M" << std::endl
    << "@" << value << std::endl
    << "A=D+A" << std::endl
    << "D=M" << std::endl
    << "@SP" << std::endl
    << "A=M" << std::endl
    << "M=D" << std::endl
    << "@SP" << std::endl
    << "M=M+1" << std::endl;
}

void Pop(std::iostream &outputFile, Segment segment, const std::string &value) {
  std::string segmentAddress;
  switch (segment) {
    case Segment::LOCAL:
      segmentAddress = "LCL";
      break;
    case Segment::ARGUMENT:
      segmentAddress = "ARG";
      break;
    case Segment::THIS:
      segmentAddress = "THIS";
      break;
    case Segment::THAT:
      segmentAddress = "THAT";
      break;
    case Segment::TMP:
      outputFile << "@SP" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << 5 + std::stoi(value) << std::endl
        << "M=D" << std::endl;
      return;
    case Segment::POINTER:
      outputFile << "@SP" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << 3 + std::stoi(value) << std::endl
        << "M=D" << std::endl;
      return;
    case Segment::STATIC:
      outputFile << "@SP" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << value << std::endl
        << "M=D" << std::endl;
      return;
    case Segment::VALUE:
      outputFile << "@SP" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << value << std::endl
        << "M=D" << std::endl;
      return;
    case Segment::FRAME:
      outputFile << "@TRANSLATOR_frame" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << value << std::endl
        << "M=D" << std::endl;
      return;
    default:
      return;
  }

  outputFile << "@" << value << std::endl
    << "D=A" << std::endl
    << "@" << segmentAddress << std::endl
    << "M=D+M" << std::endl
    << "@SP" << std::endl
    << "AM=M-1" << std::endl
    << "D=M" << std::endl
    << "@" << segmentAddress << std::endl
    << "A=M" << std::endl
    << "M=D" << std::endl
    << "@" << value << std::endl
    << "D=A" << std::endl
    << "@" << segmentAddress << std::endl
    << "M=M-D" << std::endl;
}

void Call(
    std::iostream &outputFile, const std::string &label, Word nArgs,
    Word &returnNum) {
  std::string returnLabel = "TRANSLATOR_RETURN" + std::to_string(returnNum++);
  Push(outputFile, Segment::CONSTANT, returnLabel);
  Push(outputFile, Segment::VALUE, "LCL");
  Push(outputFile, Segment::VALUE, "ARG");
  Push(outputFile, Segment::VALUE, "THIS");
  Push(outputFile, Segment::VALUE, "THAT");
  outputFile << "@SP" << std::endl
    << "D=M" << std::endl
    << "@LCL" << std::endl
    << "M=D" << std::endl
    << "@" << 5 + nArgs << std::endl
    << "D=D-A" << std::endl
    << "@ARG" << std::endl
    << "M=D" << std::endl
    << "@" << label << std::endl
    << "0;JMP" << std::endl
    << "(" << returnLabel << ")" << std::endl;
}

void Return(std::iostream &outputFile) {
  outputFile << "@LCL" << std::endl
    << "D=M" << std::endl
    << "@TRANSLATOR_frame" << std::endl
    << "M=D" << std::endl
    << "@5" << std::endl
    << "A=D-A" << std::endl
    << "D=M" << std::endl
    << "@TRANSLATOR_retAddr" << std::endl
    << "M=D" << std::endl;
  Pop(outputFile, Segment::ARGUMENT, "0");
  outputFile << "@ARG" << std::endl
    << "D=M" << std::endl
    << "@SP" << std::endl
    << "M=D+1" << std::endl;
  Pop(outputFile, Segment::FRAME, "THAT");
  Pop(outputFile, Segment::FRAME, "THIS");
  Pop(outputFile, Segment::FRAME, "ARG");
  Pop(outputFile, Segment::FRAME, "LCL");
  outputFile << "@TRANSLATOR_retAddr" << std::endl
    << "A=M" << std::endl
    << "0;JMP" << std::endl;
}

void Translate(
    std::iostream &outputFile, const std::string &inputFileName,
    std::iostream &inputFile, Word &labelNum, Word &returnNum) {
  std::string inputClass = std::filesystem::path(inputFileName).filename().string() + ".";

  std::string line;
  std::size_t lineNum = 0;
  while (std::getline(inputFile, line)) {
    ++lineNum;
    if (line.front() == '/') continue;

    std::stringstream input(line);
    std::string cmd;
    input >> cmd;

    if (cmd == "add") {
      BinaryOperation(outputFile, '+');
    } else if (cmd == "sub") {
      BinaryOperation(outputFile, '-');
    } else if (cmd == "neg") {
      UnaryOperation(outputFile, '-');
    } else if (cmd == "eq") {
      CompareOperation(outputFile, "JEQ", labelNum);
    } else if (cmd == "gt") {
      CompareOperation(outputFile, "JGT", labelNum);
    } else if (cmd == "lt") {
      CompareOperation(outputFile, "JLT", labelNum);
    } else if (cmd == "and") {
      BinaryOperation(outputFile, '&');
    } else if (cmd == "or") {
      BinaryOperation(outputFile, '|');
    } else if (cmd == "not") {
      UnaryOperation(outputFile, '!');
    } else if (cmd == "push") {
      std::string segment, value;
      input >> segment >> value;

      if (segment == "constant") {
        Push(outputFile, Segment::CONSTANT, value);
      } else if (segment == "local") {
        Push(outputFile, Segment::LOCAL, value);
      } else if (segment == "argument") {
        Push(outputFile, Segment::ARGUMENT, value);
      } else if (segment == "this") {
        Push(outputFile, Segment::THIS, value);
      } else if (segment == "that") {
        Push(outputFile, Segment::THAT, value);
      } else if (segment == "temp") {
        Push(outputFile, Segment::TMP, value);
      } else if (segment == "pointer") {
        Push(outputFile, Segment::POINTER, value);
      } else if (segment == "static") {
        Push(outputFile, Segment::STATIC, inputClass + value);
      } else {
        throw std::runtime_error("Line " + std::to_string(lineNum) +
            ": Pushing into invalid segment \"" + cmd + "\"");
      }
    } else if (cmd == "pop") {
      std::string segment, value;
      input >> segment >> value;

      if (segment == "local") {
        Pop(outputFile, Segment::LOCAL, value);
      } else if (segment == "argument") {
        Pop(outputFile, Segment::ARGUMENT, value);
      } else if (segment == "this") {
        Pop(outputFile, Segment::THIS, value);
      } else if (segment == "that") {
        Pop(outputFile, Segment::THAT, value);
      } else if (segment == "temp") {
        Pop(outputFile, Segment::TMP, value);
      } else if (segment == "pointer") {
        Pop(outputFile, Segment::POINTER, value);
      } else if (segment == "static") {
        Pop(outputFile, Segment::STATIC, inputClass + value);
      } else {
        throw std::runtime_error("Line " + std::to_string(lineNum) +
            ": Popping out of invalid segment \"" + cmd + "\"");
      }
    } else if (cmd == "label") {
      std::string label;
      input >> label;
      outputFile << "(" << label << ")" << std::endl;
    } else if (cmd == "goto") {
      std::string label;
      input >> label;
      outputFile << "@" << label << std::endl
        << "0;JMP" << std::endl;
    } else if (cmd == "if-goto") {
      std::string label;
      input >> label;
      outputFile << "@SP" << std::endl
        << "AM=M-1" << std::endl
        << "D=M" << std::endl
        << "@" << label << std::endl
        << "D;JNE" << std::endl;
    } else if (cmd == "function") {
      std::string label;
      Word nVars;
      input >> label >> nVars;
      outputFile << "(" << label << ")" << std::endl;
      while (nVars--) Push(outputFile, Segment::CONSTANT, "0");
    } else if (cmd == "call") {
      std::string label;
      Word nArgs;
      input >> label >> nArgs;
      Call(outputFile, label, nArgs, returnNum);
    } else if (cmd == "return") {
      Return(outputFile);
    } else if (!std::all_of(cmd.begin(), cmd.end(),
          [](char c) { return std::isspace(static_cast<unsigned char>(c)); })) {
      throw std::runtime_error("Line " + std::to_string(lineNum) +
          ": Invalid command \"" + cmd + "\"");
    }
  }
}

void Translate(
    std::iostream &outputFile, const std::string &inputFilename,
    Word &labelNum, Word &returnNum) {
  std::fstream inputFile(inputFilename, std::ios::in);
  Translate(outputFile, inputFilename, inputFile, labelNum, returnNum);
}

void Bootstrap(std::iostream &outputFile, bool init, Word &returnNum) {
  outputFile << "@256" << std::endl
    << "D=A" << std::endl
    << "@SP" << std::endl
    << "M=D" << std::endl
    << "@LCL" << std::endl
    << "M=D" << std::endl;
  if (init) Call(outputFile, "Sys.init", 0, returnNum);
  else Call(outputFile, "Main.main", 0, returnNum);
}

Translator::Translator() {
  clear();
}

void Translator::translateFile(
    const std::string &filename, const std::string &fileContents) {
  std::string vmFilename = filename;
  size_t pos = vmFilename.find(".jack");
  vmFilename.replace(pos, 5, ".vm");

  auto ss = std::stringstream(fileContents);
  Translate(ss_, vmFilename, ss, labelNum_, returnNum_);
}

std::string Translator::getAssembly() {
  return ss_.str();
}

void Translator::clear() {
  ss_.str("");
  labelNum_ = 0;
  returnNum_ = 0;
  Bootstrap(ss_, false, returnNum_);
}

