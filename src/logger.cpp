#include "logger.hpp"
#include <iostream>
#include <vector>

namespace Logger {
    const std::vector<std::string> log_strings = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL"
    };

    void log(LogLevel lvl, const std::string& msg, bool newline) {
        bool print = (lvl != DEBUG);
#ifdef _DEBUG
        print = true;
#endif
        if (!print) return;
        std::cout << "[" + log_strings[lvl] + "] " << msg;
        if (newline) std::cout << "\n";
    }

    template<typename T>
    void log(LogLevel lvl, T begin, T end) {
        bool print = (lvl != DEBUG);
#ifdef _DEBUG
        print = true;
#endif
        if (!print) return;
        std::cout << "[";
        for (auto it = begin; it != end; ++it) {
            std::cout << *it;
            if (std::next(it) != end) std::cout << " ";
        }
        std::cout << "]\n";
    }
}