#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <stdexcept>
#include <sstream>

namespace Assembler {

using Word = std::int16_t;
constexpr int WORD_WIDTH = 16;

void PrintBinary(std::iostream &out, Word x, int bits = WORD_WIDTH - 1) {
  std::string s;
  while (bits--)
    out << ((x >> bits) & 1);
}

std::unordered_map<std::string, Word> baseSymbolTable;
std::unordered_map<std::string, std::string> destinations;
std::unordered_map<std::string, std::string> computations;
std::unordered_map<std::string, std::string> jumps;
void initialize() {
  baseSymbolTable["SP"] =     0;
  baseSymbolTable["LCL"] =    1;
  baseSymbolTable["ARG"] =    2;
  baseSymbolTable["THIS"] =   3;
  baseSymbolTable["THAT"] =   4;
  baseSymbolTable["SCREEN"] = 16384;
  baseSymbolTable["KBD"] =    24576;
  for (Word i = 0; i < 16; ++i)
    baseSymbolTable["R" + std::to_string(i)] = i;

  destinations[""]    = "000";
  destinations["M"]   = "001";
  destinations["D"]   = "010";
  destinations["MD"]  = "011";
  destinations["A"]   = "100";
  destinations["AM"]  = "101";
  destinations["AD"]  = "110";
  destinations["AMD"] = "111";

  computations["0"]   = "0101010"; // same
  computations["1"]   = "0111111"; // same
  computations["-1"]  = "0111010"; //
  computations["D"]   = "0001100";
  computations["A"]   = "0110000";
  computations["!D"]  = "0001101";
  computations["!A"]  = "0110001";
  computations["-D"]  = "0001111"; //
  computations["-A"]  = "0110011"; //
  computations["D+1"] = "0011111"; //
  computations["A+1"] = "0110111"; //
  computations["D-1"] = "0001110"; //
  computations["A-1"] = "0110010"; //
  computations["D+A"] = "0000010"; //
  computations["D-A"] = "0010011"; //
  computations["A-D"] = "0000111"; //
  computations["D&A"] = "0000000";
  computations["D|A"] = "0010101";
  computations["M"]   = "1110000";
  computations["!M"]  = "1110001";
  computations["-M"]  = "1110011"; // same as -A
  computations["M+1"] = "1110111"; // same as A+1
  computations["M-1"] = "1110010"; // same as A-1
  computations["D+M"] = "1000010"; // same as D+A
  computations["D-M"] = "1010011"; // same as D-A
  computations["M-D"] = "1000111"; // same as A-D
  computations["D&M"] = "1000000";
  computations["D|M"] = "1010101";

  jumps[""]     = "000";
  jumps["JGT"]  = "001";
  jumps["JEQ"]  = "010";
  jumps["JGE"]  = "011";
  jumps["JLT"]  = "100";
  jumps["JNE"]  = "101";
  jumps["JLE"]  = "110";
  jumps["JMP"]  = "111";
}

bool IsNumber(const std::string &str) {
  for (const auto &c : str)
    if (!std::isdigit(c)) return false;
  return true;
}

std::string assemble(const std::string &assembly) {
  std::stringstream inputFile(assembly);
  std::stringstream outputFile;

  std::string line;

  std::unordered_map<std::string, Word> symbolTable(baseSymbolTable);
  std::unordered_map<std::string, Word> labelTable;
  Word hackLineNumber = 0;
  Word symbolNumber = 16;
  int asmLineNumber = 0;
  while (std::getline(inputFile, line)) {
    std::stringstream input(line);

    char front;
    input >> std::ws >> front;

    if (front == '(') {
      // Label
      std::string label;
      std::getline(input, label, ')');
      labelTable[label] = hackLineNumber;
    } else if (front != '/') {
      // A or C instruction
      ++hackLineNumber;
    }
  }

  inputFile.clear();
  inputFile.seekg(std::ios::beg);
  asmLineNumber = 0;
  while (std::getline(inputFile, line)) {
    ++asmLineNumber;
    std::stringstream input(line);

    input >> std::ws;
    std::getline(input, line);
    input = std::stringstream(line);
    char front;
    input >> front;

    if (front == '@') {
      // A instruction
      outputFile << "0";

      std::string addressStr;
      input >> addressStr;

      Word address;
      if (IsNumber(addressStr)) {
        address = static_cast<Word>(std::stoi(addressStr));
      } else {
        auto addressIt = labelTable.find(addressStr);
        if (addressIt == labelTable.end()) {
          addressIt = symbolTable.find(addressStr);
          if (addressIt == symbolTable.end()) {
            symbolTable[addressStr] = address = symbolNumber++;
          } else {
            address = addressIt->second;
          }
        } else {
          address = addressIt->second;
        }
      }
      PrintBinary(outputFile, address);
      outputFile << std::endl;
    } else if (front != '(' && front != '/' && !std::isspace(front)) {
      // C instruction
      for (int i = 0; i < WORD_WIDTH - 13; ++i)
        outputFile << "1";

      while (std::isspace(line.back()))
        line.pop_back();

      auto destPos = line.find('=');
      std::string destination;
      if (destPos == std::string::npos) {
        // No destination
        destPos = 0;
        destination = destinations[""];
      } else {
        // Destination
        auto destStr = line.substr(0, destPos++);
        auto destIt = destinations.find(destStr);
        if (destIt == destinations.end())
          throw std::runtime_error("Line " + std::to_string(asmLineNumber) +
              ": invalid destination " + destStr);
        destination = destIt->second;
      }

      auto compPos = line.find(';');
      if (compPos == std::string::npos) {
        // Compuation (no jump)
        auto compStr = line.substr(destPos);
        compStr = compStr.substr(0, compStr.find_first_of(' '));
        auto compIt = computations.find(compStr);
        if (compIt == computations.end())
          throw std::runtime_error("Line " + std::to_string(asmLineNumber) +
              ": invalid computation " + compStr);
        outputFile << compIt->second;

        outputFile << destination;

        outputFile << jumps[""];
      } else {
        // Compuation and jump
        auto compStr = line.substr(destPos, compPos - destPos);
        auto compIt = computations.find(compStr);
        if (compIt == computations.end())
          throw std::runtime_error("Line " + std::to_string(asmLineNumber) +
              ": invalid computation " + compStr);
        outputFile << compIt->second;

        outputFile << destination;

        auto jumpStr = line.substr(compPos + 1);
        jumpStr = jumpStr.substr(0, jumpStr.find_first_of(' '));
        auto jumpIt = jumps.find(jumpStr);
        if (jumpIt == jumps.end())
          throw std::runtime_error("Line " + std::to_string(asmLineNumber) +
              ": invalid jump " + jumpStr);
        outputFile << jumpIt->second;
      }
      outputFile << std::endl;
    }
  }

  return outputFile.str();
}

} // namespace Assembler
