#include "command.h"

Command::Command()
: Command(0) {
}

Command::Command(size_t args_size)
: args_size{args_size} {
}

bool Command::addArgs(std::string&& s) {
  if (isEmpty() || isComplete()) return false;
  args.push_back(std::move(s));
  return true;
}

void Command::reset() {
  args_size = 0;
  args.clear();
}

bool Command::isEmpty() {
  return args_size == 0;
}

bool Command::isComplete() {
  return !isEmpty() && args.size() == args_size;
}