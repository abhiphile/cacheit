#include <string>
#include <vector>

#pragma once

struct Command {
  Command();
  Command(size_t args_size);

  bool addArgs(std::string&& s);
  void reset();
  bool isEmpty();
  bool isComplete();

  size_t args_size{};
  std::vector<std::string> args;
};