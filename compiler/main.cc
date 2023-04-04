#include <iostream>
#include <fstream>
#include "driver.hh"

int main(int argc, char **argv) {
  Driver driver;
  for (int i = 1; i < argc; ++i) {
    auto code = driver.parse(argv[i]);

    std::string filename = argv[i];
    filename.replace(filename.find_last_of("."), std::string::npos, ".jack");
    std::fstream file(filename, std::ios::out);
    file << code;
  }
}
