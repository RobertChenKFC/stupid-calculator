#include "vmcommand.h"

#include <iostream>
#include <climits>
#include <sstream>

VMCommand::VMCommand(Operation op) :
  op_(op)
{

}

VMCommand::VMCommand(Operation op, Segment segment, Word offset) :
  op_(op),
  segment_(segment),
  n_(offset)
{

}

VMCommand::VMCommand(Operation op, const std::string &label) :
  op_(op),
  str_(label)
{

}

VMCommand::VMCommand(Operation op, const std::string &func, Word n) :
  op_(op),
  str_(func),
  n_(n)
{

}

void VMCommand::toVMCode(std::iostream &outputFile) const {
  switch (op_) {
    case VMCommand::Operation::ADD:
      outputFile << "add" << std::endl;
      break;
    case VMCommand::Operation::SUB:
      outputFile << "sub" << std::endl;
      break;
    case VMCommand::Operation::NEG:
      outputFile << "neg" << std::endl;
      break;
    case VMCommand::Operation::EQ:
      outputFile << "eq" << std::endl;
      break;
    case VMCommand::Operation::GT:
      outputFile << "gt" << std::endl;
      break;
    case VMCommand::Operation::LT:
      outputFile << "lt" << std::endl;
      break;
    case VMCommand::Operation::AND:
      outputFile << "and" << std::endl;
      break;
    case VMCommand::Operation::OR:
      outputFile << "or" << std::endl;
      break;
    case VMCommand::Operation::NOT:
      outputFile << "not" << std::endl;
      break;
    case VMCommand::Operation::POP:
      outputFile << "pop " << TO_SEGMENT_STRING(segment_) << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::PUSH:
      outputFile << "push " << TO_SEGMENT_STRING(segment_) << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::LABEL:
      outputFile << "label " << str_ << std::endl;
      break;
    case VMCommand::Operation::GOTO:
      outputFile << "goto " << str_ << std::endl;
      break;
    case VMCommand::Operation::IF_GOTO:
      outputFile << "if-goto " << str_ << std::endl;
      break;
    case VMCommand::Operation::FUNCTION:
      outputFile << "function " << str_ << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::CALL:
      outputFile << "call " << str_  << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::RETURN:
      outputFile << "return" << std::endl;
      break;
    default:
      break;
  }
}

void VMCommand::print() const {
  switch (op_) {
    case VMCommand::Operation::ADD:
      std::cout << "add" << std::endl;
      break;
    case VMCommand::Operation::SUB:
      std::cout << "sub" << std::endl;
      break;
    case VMCommand::Operation::NEG:
      std::cout << "neg" << std::endl;
      break;
    case VMCommand::Operation::EQ:
      std::cout << "eq" << std::endl;
      break;
    case VMCommand::Operation::GT:
      std::cout << "gt" << std::endl;
      break;
    case VMCommand::Operation::LT:
      std::cout << "lt" << std::endl;
      break;
    case VMCommand::Operation::AND:
      std::cout << "and" << std::endl;
      break;
    case VMCommand::Operation::OR:
      std::cout << "or" << std::endl;
      break;
    case VMCommand::Operation::NOT:
      std::cout << "not" << std::endl;
      break;
    case VMCommand::Operation::POP:
      std::cout << "pop " << TO_SEGMENT_STRING(segment_) << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::PUSH:
      std::cout << "push " << TO_SEGMENT_STRING(segment_) << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::LABEL:
      std::cout << "label " << str_ << std::endl;
      break;
    case VMCommand::Operation::GOTO:
      std::cout << "goto " << str_ << std::endl;
      break;
    case VMCommand::Operation::IF_GOTO:
      std::cout << "if-goto " << str_ << std::endl;
      break;
    case VMCommand::Operation::FUNCTION:
      std::cout << "function " << str_ << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::CALL:
      std::cout << "call " << str_  << ' ' << n_ << std::endl;
      break;
    case VMCommand::Operation::RETURN:
      std::cout << "return" << std::endl;
      break;
    default:
      break;
  }
}

VMCommands::VMCommands() :
  initializedExecution_(false)
{

}


VMCommands::VMCommands(const std::string &inputFileName) :
  memory_(nullptr),
  initializedExecution_(false)
{
  if (!initialized_) throw std::runtime_error("Please call VMCommands::init before using VMCommands!");

  std::fstream inputFile(inputFileName, std::ios::in);

  std::string str;
  while (inputFile >> str) {
    VMCommand::Operation op = operations_[str];
    if (op == VMCommand::Operation::PUSH || op == VMCommand::Operation::POP) {
      Word offset;
      inputFile >> str >> offset;
      VMCommand::Segment segment = segments_[str];
      vmCommands_.push_back(VMCommand(op, segment, offset));
    } else if (op == VMCommand::Operation::LABEL ||
        op == VMCommand::Operation::GOTO || op == VMCommand::Operation::IF_GOTO) {
      inputFile >> str;
      vmCommands_.push_back(VMCommand(op, str));
    } else if (op == VMCommand::Operation::FUNCTION || op == VMCommand::Operation::CALL) {
      Word n;
      inputFile >> str >> n;
      vmCommands_.push_back(VMCommand(op, str, n));
    } else {
      vmCommands_.push_back(VMCommand(op));
    }
  }

  inputFile.close();
}

bool VMCommands::initialized_ = false;
std::unordered_map<std::string, VMCommand::Operation> VMCommands::operations_;
std::unordered_map<std::string, VMCommand::Segment> VMCommands::segments_;
void VMCommands::init() {
  if (initialized_) return;

  initialized_ = true;

  operations_["add"] = VMCommand::Operation::ADD;
  operations_["sub"] = VMCommand::Operation::SUB;
  operations_["neg"] = VMCommand::Operation::NEG;
  operations_["eq"] = VMCommand::Operation::EQ;
  operations_["gt"] = VMCommand::Operation::GT;
  operations_["lt"] = VMCommand::Operation::LT;
  operations_["and"] = VMCommand::Operation::AND;
  operations_["or"] = VMCommand::Operation::OR;
  operations_["not"] = VMCommand::Operation::NOT;
  operations_["push"] = VMCommand::Operation::PUSH;
  operations_["pop"] = VMCommand::Operation::POP;
  operations_["label"] = VMCommand::Operation::LABEL;
  operations_["goto"] = VMCommand::Operation::GOTO;
  operations_["if-goto"] = VMCommand::Operation::IF_GOTO;
  operations_["function"] = VMCommand::Operation::FUNCTION;
  operations_["call"] = VMCommand::Operation::CALL;
  operations_["return"] = VMCommand::Operation::RETURN;

  segments_["static"] = VMCommand::Segment::STATIC;
  segments_["this"] = VMCommand::Segment::THIS;
  segments_["local"] = VMCommand::Segment::LOCAL;
  segments_["argument"] = VMCommand::Segment::ARGUMENT;
  segments_["that"] = VMCommand::Segment::THAT;
  segments_["constant"] = VMCommand::Segment::CONSTANT;
  segments_["pointer"] = VMCommand::Segment::POINTER;
  segments_["temp"] = VMCommand::Segment::TEMP;
}

void VMCommands::add(const VMCommand &vmCommand) {
  vmCommands_.push_back(vmCommand);
}

void VMCommands::add(VMCommand &&vmCommand) {
  vmCommands_.push_back(vmCommand);
}

void VMCommands::add(const VMCommands &vmCommands) {
  std::size_t begin = vmCommands_.size();
  vmCommands_.reserve(vmCommands_.size() + vmCommands.size());
  vmCommands_.insert(vmCommands_.end(), vmCommands.begin(), vmCommands.end());
  Word maxStatic = -1;
  std::string functionName;
  for (std::size_t i = begin; i < vmCommands_.size(); ++i) {
    auto &vmCommand = vmCommands_[i];
    if (vmCommand.op_ == VMCommand::Operation::FUNCTION) {
      functionName = vmCommand.str_;
    } else if (vmCommand.op_ == VMCommand::Operation::LABEL || vmCommand.op_ == VMCommand::Operation::GOTO ||
        vmCommand.op_ == VMCommand::Operation::IF_GOTO) {
      vmCommand.str_ = functionName + vmCommand.str_;
    } else if ((vmCommand.op_ == VMCommand::Operation::PUSH || vmCommand.op_ == VMCommand::Operation::POP) &&
        vmCommand.segment_ == VMCommand::Segment::STATIC) {
      if (vmCommand.n_ > maxStatic)
        maxStatic = vmCommand.n_;
      vmCommand.n_ += numStatics_;
    }
  }
  numStatics_ += maxStatic + 1;
}

void VMCommands::add(VMCommands &&vmCommands) {
  std::size_t begin = vmCommands_.size();
  vmCommands_.reserve(vmCommands_.size() + vmCommands.size());
  std::move(vmCommands.begin(), vmCommands.end(), std::back_inserter(vmCommands_));
  Word maxStatic = -1;
  std::string functionName;
  for (std::size_t i = begin; i < vmCommands_.size(); ++i) {
    auto &vmCommand = vmCommands_[i];
    if (vmCommand.op_ == VMCommand::Operation::FUNCTION) {
      functionName = vmCommand.str_;
    } else if (vmCommand.op_ == VMCommand::Operation::LABEL || vmCommand.op_ == VMCommand::Operation::GOTO ||
        vmCommand.op_ == VMCommand::Operation::IF_GOTO) {
      vmCommand.str_ = functionName + vmCommand.str_;
    } else if ((vmCommand.op_ == VMCommand::Operation::PUSH || vmCommand.op_ == VMCommand::Operation::POP) &&
        vmCommand.segment_ == VMCommand::Segment::STATIC) {
      if (vmCommand.n_ > maxStatic)
        maxStatic = vmCommand.n_;
      vmCommand.n_ += numStatics_;
    }
  }
  numStatics_ += maxStatic + 1;
}

std::vector<VMCommand>::iterator VMCommands::begin() {
  return vmCommands_.begin();
}

std::vector<VMCommand>::const_iterator VMCommands::begin() const {
  return vmCommands_.begin();
}

std::vector<VMCommand>::iterator VMCommands::end() {
  return vmCommands_.end();
}

std::vector<VMCommand>::const_iterator VMCommands::end() const {
  return vmCommands_.end();
}

VMCommand& VMCommands::operator[](std::size_t idx) {
  return vmCommands_[idx];
}

const VMCommand& VMCommands::operator[](std::size_t idx) const {
  return vmCommands_[idx];
}

std::size_t VMCommands::size() const {
  return vmCommands_.size();
}

void VMCommands::setMemoryPtr(Word *ptr) {
  memory_ = ptr;
}

void VMCommands::setKey(Word key) {
  memory_[KBD] = key;
}

void VMCommands::initExecution() {
  if (initializedExecution_) return;
  initializedExecution_ = true;

  for (std::size_t i = 0; i < 24577; ++i)
    memory_[i] = 0;
  memory_[SP] = memory_[LCL] = STACK;

  std::unordered_map<std::string, std::size_t> functionTable;
  std::unordered_map<std::string, std::size_t> labelTable;
  for (std::size_t pos = 0; pos < vmCommands_.size(); ++pos) {
    const auto &vmCommand = vmCommands_[pos];
    if (vmCommand.op_ == VMCommand::Operation::FUNCTION) {
      functionTable[vmCommand.str_] = pos;
    } else if (vmCommand.op_ == VMCommand::Operation::LABEL) {
      labelTable[vmCommand.str_] = pos;
    }
  }
  for (auto &vmCommand : vmCommands_) {
    if (vmCommand.op_ == VMCommand::Operation::CALL) {
      auto it = functionTable.find(vmCommand.str_);
      if (it == functionTable.end())
        throw std::runtime_error("Function \"" + vmCommand.str_ + "\" is undefined");
      vmCommand.label_ = static_cast<Word>(it->second);
    } else if (vmCommand.op_ == VMCommand::Operation::GOTO || vmCommand.op_ == VMCommand::Operation::IF_GOTO) {
      auto it = labelTable.find(vmCommand.str_);
      if (it == labelTable.end())
        throw std::runtime_error("Label \"" + vmCommand.str_ + "\" is undefined");
      vmCommand.label_ = static_cast<Word>(it->second);
    }
  }

  auto it = functionTable.find("Sys.init");
  if (it == functionTable.end())
    throw std::runtime_error("Cannot find entry point Sys.init");
  startPos_ = curPos_ = it->second;
}

bool VMCommands::execute(std::size_t steps) {
  if (memory_ == nullptr)
    throw std::runtime_error("Please call VMCommands::setMemoryPtr before executing!");
  if (!initializedExecution_)
    throw std::runtime_error("Please call VMCommands::initExecution() before executing!");

  std::size_t pos = curPos_;
  for (std::size_t curStep = 0; curStep < steps; ++curStep) {
    if (curPos_ >= vmCommands_.size())
      throw std::runtime_error("Out of program memory");

    const auto &vmCommand = vmCommands_[pos];

    bool jump = false;
    Word currentLCL;
    std::size_t returnPos;
    switch (vmCommand.op_) {
      case VMCommand::Operation::ADD:
        memory_[memory_[SP] - 2] =
            static_cast<Word>(static_cast<int16_t>(
                static_cast<int16_t>(memory_[memory_[SP] - 2]) +
                static_cast<int16_t>(memory_[memory_[SP] - 1])));
        --memory_[SP];
        break;
      case VMCommand::Operation::SUB:
        memory_[memory_[SP] - 2] =
            static_cast<Word>(static_cast<int16_t>(
                static_cast<int16_t>(memory_[memory_[SP] - 2]) -
                static_cast<int16_t>(memory_[memory_[SP] - 1])));
        --memory_[SP];
        break;
      case VMCommand::Operation::NEG:
        memory_[memory_[SP] - 1] = -memory_[memory_[SP] - 1];
        break;
      case VMCommand::Operation::EQ:
        memory_[memory_[SP] - 2] = (memory_[memory_[SP] - 2] == memory_[memory_[SP] - 1]) ? (-1) : 0;
        --memory_[SP];
        break;
      case VMCommand::Operation::GT:
        memory_[memory_[SP] - 2] = (memory_[memory_[SP] - 2] > memory_[memory_[SP] - 1]) ? (-1) : 0;
        --memory_[SP];
        break;
      case VMCommand::Operation::LT:
        memory_[memory_[SP] - 2] = (memory_[memory_[SP] - 2] < memory_[memory_[SP] - 1]) ? (-1) : 0;
        --memory_[SP];
        break;
      case VMCommand::Operation::AND:
        memory_[memory_[SP] - 2] = memory_[memory_[SP] - 2] & memory_[memory_[SP] - 1];
        --memory_[SP];
        break;
      case VMCommand::Operation::OR:
        memory_[memory_[SP] - 2] = memory_[memory_[SP] - 2] | memory_[memory_[SP] - 1];
        --memory_[SP];
        break;
      case VMCommand::Operation::NOT:
        memory_[memory_[SP] - 1] = ~memory_[memory_[SP] - 1];
        break;
      case VMCommand::Operation::POP:
        pop_(vmCommand.segment_, vmCommand.n_);
        break;
      case VMCommand::Operation::PUSH:
        push_(vmCommand.segment_, vmCommand.n_);
        break;
      case VMCommand::Operation::GOTO:
        jump = true;
        pos = static_cast<std::size_t>(vmCommand.label_);
        break;
      case VMCommand::Operation::IF_GOTO:
        if (memory_[memory_[SP] - 1])
          pos = static_cast<std::size_t>(vmCommand.label_);
        --memory_[SP];
        break;
      case VMCommand::Operation::FUNCTION:
        for (Word k = 0; k < vmCommand.n_; ++k)
          push_(VMCommand::Segment::CONSTANT, 0);
        break;
      case VMCommand::Operation::CALL:
        push_(VMCommand::Segment::CONSTANT, static_cast<Word>(pos + 1));
        push_(VMCommand::Segment::CONSTANT, memory_[LCL]);
        push_(VMCommand::Segment::CONSTANT, memory_[ARG]);
        push_(VMCommand::Segment::CONSTANT, memory_[THIS]);
        push_(VMCommand::Segment::CONSTANT, memory_[THAT]);
        memory_[ARG] = memory_[SP] - vmCommand.n_ - 5;
        memory_[LCL] = memory_[SP];

        jump = true;
        if (vmCommand.str_ == "Sys.halt") {
          return true;
        }
        pos = static_cast<std::size_t>(vmCommand.label_);
        break;
      case VMCommand::Operation::RETURN:
        currentLCL = memory_[LCL];
        returnPos = static_cast<std::size_t>(memory_[currentLCL - 5]);
        pop_(VMCommand::Segment::ARGUMENT, 0);
        memory_[SP] = memory_[ARG] + 1;
        memory_[THAT] = memory_[currentLCL - 1];
        memory_[THIS] = memory_[currentLCL - 2];
        memory_[ARG] = memory_[currentLCL - 3];
        memory_[LCL] = memory_[currentLCL - 4];

        jump = true;
        pos = returnPos;
        break;
      default:
        break;
    }
    if (!jump) ++pos;
  }

  curPos_ = pos;
  return false;
}

void VMCommands::reset() {
  curPos_ = startPos_;
  for (std::size_t i = 0; i < 24577; ++i)
    memory_[i] = 0;
  memory_[SP] = memory_[LCL] = STACK;
}

Word VMCommands::numStatics_ = 0;
void VMCommands::clear() {
  initializedExecution_ = false;
  numStatics_ = 0;
  vmCommands_.clear();
}

void VMCommands::push_(VMCommand::Segment segment, Word offset) const {
  switch (segment) {
    case VMCommand::Segment::STATIC:
      memory_[memory_[SP]] = memory_[STATIC + offset];
      break;
    case VMCommand::Segment::THIS:
      memory_[memory_[SP]] = memory_[memory_[THIS] + offset];
      break;
    case VMCommand::Segment::LOCAL:
      memory_[memory_[SP]] = memory_[memory_[LCL] + offset];
      break;
    case VMCommand::Segment::ARGUMENT:
      memory_[memory_[SP]] = memory_[memory_[ARG] + offset];
      break;
    case VMCommand::Segment::THAT:
      memory_[memory_[SP]] = memory_[memory_[THAT] + offset];
      break;
    case VMCommand::Segment::CONSTANT:
      memory_[memory_[SP]] = offset;
      break;
    case VMCommand::Segment::POINTER:
      memory_[memory_[SP]] = memory_[THIS + offset];
      break;
    case VMCommand::Segment::TEMP:
      memory_[memory_[SP]] = memory_[TEMP + offset];
      break;
    default:
      break;
  }
  ++memory_[SP];
}

void VMCommands::pop_(VMCommand::Segment segment, Word offset) const {
  --memory_[SP];
  switch (segment) {
    case VMCommand::Segment::STATIC:
      memory_[STATIC + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::THIS:
      memory_[memory_[THIS] + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::LOCAL:
      memory_[memory_[LCL] + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::ARGUMENT:
      memory_[memory_[ARG] + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::CONSTANT:
      throw std::runtime_error("Cannot pop into constant memory segment");
    case VMCommand::Segment::THAT:
      memory_[memory_[THAT] + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::POINTER:
      memory_[THIS + offset] = memory_[memory_[SP]];
      break;
    case VMCommand::Segment::TEMP:
      memory_[TEMP + offset] = memory_[memory_[SP]];
      break;
    default:
      break;
  }
}

void VMCommands::toVMCode(std::iostream &outputFile) const {
  for (const auto &vmCommand : vmCommands_)
    vmCommand.toVMCode(outputFile);
}

std::string VMCommands::toVMCodeString() const {
  std::stringstream ss;
  toVMCode(ss);
  return ss.str();
}

