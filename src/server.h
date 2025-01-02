#include <cstring>
#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "command.h"
#include "conn.h"
#include "epoll_manager.h"
#include "utils.h"

#pragma once

class RedisServer {
public:
  RedisServer();
  void startTcpServer(int port);
  void runServer();
  int acceptConn();
  // Command parseRequest(Conn& c);
  std::pair<std::string, int> execSet(Conn& c);
  std::pair<std::string, int> execGet(Conn& c);
  std::pair<std::string, int> execDel(Conn& c);
  int execCommand(Conn& c);
  int handleRead(Conn& c);
  int handleWrite(Conn& c);

private:
  int server_fd;
  EpollManager epoll_man{};
  std::unordered_map<std::string, std::string> kvstore;
  std::vector<Conn> clients;
};