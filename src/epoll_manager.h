#include <stdio.h>
#include <stdlib.h>

#include <sys/epoll.h>
#include <vector>
#include "utils.h"

#pragma once

class EpollManager {
public:
  EpollManager();
  int register_fd(int fd, uint32_t flags);
  int mod_fd(int fd, uint32_t flags);
  int delete_fd(int fd);
  int poll(std::vector<struct epoll_event>& events);
private:
  int epoll_fd;
};