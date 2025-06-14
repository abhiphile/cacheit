// filepath: /cacheit/cacheit/include/command.hpp
#pragma once

#include <string>
#include <vector>

class Command {
public:
    Command();
    Command(size_t args_size);

    bool addArgs(std::string&& s);
    void reset();
    bool isEmpty() const;
    bool isComplete() const;
    size_t getArgsSize() const { return args_size; }
    const std::vector<std::string>& getArgs() const { return args; }

private:
    size_t args_size{};
    std::vector<std::string> args;
};