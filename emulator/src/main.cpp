#include "compiler/tokenizer.h"
#include "compiler/parser.h"
#include "compiler/vmcommand.h"
#include "translator/translator.h"
#include "assembler/assembler.h"

#include <string>
#include <exception>
#include <iostream>
#include <chrono>
#include <emscripten.h>

using Word = int16_t;
constexpr int WORD_WIDTH = 16;

#define IS_A_INSTRUCTION(instruction)   ~(((instruction) >> (WORD_WIDTH - 1)))
#define IS_C_INSTRUCTION(instruction)   ((instruction) >> (WORD_WIDTH - 1))


#define USE_REGISTER_M(instruction)     (((instruction) >> 12) & 1)

#define ZERO_X(instruction)             (((instruction) >> 11) & 1)
#define NEGATE_X(instruction)           (((instruction) >> 10) & 1)
#define ZERO_Y(instruction)             (((instruction) >>  9) & 1)
#define NEGATE_Y(instruction)           (((instruction) >>  8) & 1)
#define ALU_ADD(instruction)            (((instruction) >>  7) & 1)
#define NEGATE_OUT(instruction)         (((instruction) >>  6) & 1)

#define DEST_A(instruction)             (((instruction) >>  5) & 1)
#define DEST_D(instruction)             (((instruction) >>  4) & 1)
#define DEST_M(instruction)             (((instruction) >>  3) & 1)

#define JLT(instruction)                (((instruction) >>  2) & 1)
#define JEQ(instruction)                (((instruction) >>  1) & 1)
#define JGT(instruction)                (((instruction)      ) & 1)

class Timer {
public:
  Timer(std::string msg) :
      start_(std::chrono::high_resolution_clock::now()),
      msg_(msg) {}

  ~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start_;
    std::cout << msg_ << ": " << diff.count() << " seconds\n";
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  std::string msg_;
};

extern "C" {
  constexpr size_t ROM_SIZE = 32768;
  constexpr Word KBD = 16397;

  VMCommands vmCommands;
  std::unordered_map<std::string, std::string> vmCodeStrings;
  std::string assemblyString; 

  Translator translator;
  Word program[ROM_SIZE];
  Word *memory;
  Word pc, registerA, registerD;

  void Init();
  size_t CompileFile(char *inputFileName, char *input, int length);
  void GetVmCodeString(char *inputFileName, char *vmCodeStringBuffer);
  void TranslateFile(char *inputFileName);
  size_t GetAssemblyLength();
  void GetAssembly(char *assemblyBuffer);
  void Assemble();
  void GetMachineCode(char *machineCodeBuffer, int idx);
  void SetMemoryPtr(Word *ptr);
  bool InitializeExecution();
  bool Execute(std::size_t steps);
  void SetKey(Word key);
  void Reset();
  void Clear();

  void Init() {
    Assembler::initialize();
  }

  size_t CompileFile(char *inputFileName, char *inputArray, int length) {
    try {
      std::string input(inputArray, inputArray + length);
      Tokenizer::init();

      Parser parser(inputFileName, input);
      auto curVmCommands = parser.toVMCommands();
      vmCommands.add(curVmCommands);

      auto vmCodeString = curVmCommands.toVMCodeString();
      vmCodeStrings[inputFileName] = vmCodeString;

      EM_ASM_({
        console.log('Compiled ' + UTF8ToString($0, 256));
      }, inputFileName);

      return vmCodeString.length();
    } catch (const std::exception &e) {
      EM_ASM_({
        console.log(UTF8ToString($0, 256) + ', ' +
          UTF8ToString($1, 256));
      }, inputFileName, e.what());

      return 0;
    }
  }

  void GetVmCodeString(char *inputFileName, char *vmCodeStringBuffer) {
    const auto &vmCodeString = vmCodeStrings[inputFileName];
    memcpy(
        vmCodeStringBuffer, vmCodeString.c_str(),
        sizeof(char) * vmCodeString.length());
  }

  void TranslateFile(char *inputFileName) {
    translator.translateFile(inputFileName, vmCodeStrings[inputFileName]);
    EM_ASM_({
      console.log('Translated ' + UTF8ToString($0, 256));
    }, inputFileName);
  }

  size_t GetAssemblyLength() {
    assemblyString = translator.getAssembly();
    return assemblyString.length();
  }

  void GetAssembly(char *assemblyBuffer) {
    memcpy(assemblyBuffer, assemblyString.c_str(), assemblyString.length());
  }

  void Assemble() {
    // DEBUG
    std::string machineCodeString;
    {
      Timer timer("Assemble");
      machineCodeString = Assembler::assemble(assemblyString);
    }

    // DEBUG
    {
      Timer timer("Conversion");
      memset(program, 0, sizeof(program));
      int idx = 0;
      for (size_t i = 0; i < machineCodeString.length(); i += WORD_WIDTH + 1) {
        for (size_t j = 0; j < WORD_WIDTH; ++j) {
          program[idx] |=
              static_cast<Word>(machineCodeString[i + j] - '0') <<
              (WORD_WIDTH - 1 - j);
        }
        ++idx;
      }
    }

    EM_ASM(console.log(
        'Assembled program, contains ' + UTF8ToString($0) + ' instructions');,
        std::to_string(machineCodeString.length() / WORD_WIDTH).c_str());
  }

  void GetMachineCode(char *machineCodeBuffer, int idx) {
    for (size_t i = 0; i < ROM_SIZE; ++i)
      machineCodeBuffer[i] =
          static_cast<char>((program[i] >> (idx * 8)) & 0xff);
  }

  void SetMemoryPtr(Word *ptr) {
    memory = ptr;
  }

  bool InitializeExecution() {
    pc = 0;
    return true;
  }

  bool Execute(std::size_t steps) {
    for (std::size_t step = 0; step < steps; ++step) {
      Word instruction = program[pc++];
      if (IS_A_INSTRUCTION(instruction)) {
        registerA = instruction;
      } else {
        Word x = registerD;
        Word y = USE_REGISTER_M(instruction) ?
                 memory[registerA & 0xffff] :
                 registerA;
        x = ~(-  ZERO_X(instruction)) & x;
        x =  (-NEGATE_X(instruction)) ^ x;
        y = ~(-  ZERO_Y(instruction)) & y;
        y =  (-NEGATE_Y(instruction)) ^ y;
        Word result = ALU_ADD(instruction) ? (x + y) : (x & y);
        result = (-NEGATE_OUT(instruction)) ^ result;

        if (DEST_M(instruction)) memory[registerA & 0xffff] = result;
        if (DEST_A(instruction)) registerA = result;
        if (DEST_D(instruction)) registerD = result;

        int16_t result16 = static_cast<int16_t>(result & 0xffff);
        bool jump = (JLT(instruction) && result16 <  0) ||
                    (JEQ(instruction) && result16 == 0) ||
                    (JGT(instruction) && result16 >  0);
        if (jump) pc = registerA;
      }
    }

    return false;
  }

  void SetKey(Word key) {
    // DEBUG
    // std::cout << "key: " << key << "\n";

    memory[KBD] = key;
  }

  void Reset() {
    pc = 0;
  }

  void Clear() {
    Reset();
    Parser::reset();
    vmCommands.clear();
    vmCodeStrings.clear();
    assemblyString.clear();
    translator.clear();
  }

  int main() {

  }
}

