#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#pragma once

namespace Consts {
  const size_t SIZE_HEADER_LEN = 4;
  const size_t MAX_MSG_LEN = 4096;
  const size_t MAX_EPOLL_EVENTS = 512;
}

inline int setnonblocking(int fd) {
  errno = 0;
  int flags = fcntl(fd, F_GETFL, 0);
  if (errno) {
    perror("fcntl error");
    return -1;
  }

  flags |= O_NONBLOCK;

  errno = 0;
  return fcntl(fd, F_SETFL, flags);
}

namespace Logger {
  enum LogLevel {
    DEBUG,
    INFO,
    // WARNING,
    ERROR,
    // CRITICAL
  };

  const std::vector<std::string> log_strings = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL"
  };

  inline void log(LogLevel lvl, const std::string& msg, bool newline = true) {
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
      if (next(it) != end) std::cout << " ";
    }
    std::cout << "]\n";
  }
}