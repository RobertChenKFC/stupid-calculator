#pragma once

#include <sstream>

using Word = std::int16_t;

class Translator {
public:
  Translator();
  void translateFile(
      const std::string &filename, const std::string &fileContents);
  std::string getAssembly();
  void clear();

private:
  std::stringstream ss_;
  Word labelNum_;
  Word returnNum_;
};
