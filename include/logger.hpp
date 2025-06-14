#include <string>

namespace Logger {
    enum LogLevel {
        DEBUG,
        INFO,
        ERROR
    };

    void log(LogLevel lvl, const std::string& msg, bool newline = true);
    
    template<typename T>
    void log(LogLevel lvl, T begin, T end);
}