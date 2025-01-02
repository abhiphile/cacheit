#include "server.h"

RedisServer::RedisServer() 
  : clients(10) {
}

/**
 * Start a TCP server that listens to wildcard address 0.0.0.0
 * on the specified port.
 * 
 * This function sets server_fd
 * 
 * @param[in] port the port that will be bound
 * @param[out] server_fd
 */
void RedisServer::startTcpServer(int port) {
  int gai_status;
  struct addrinfo hints{};
  struct addrinfo *addresses;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((gai_status = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addresses)) != 0) {
    Logger::log(Logger::ERROR, "getaddrinfo error: " + std::string(gai_strerror(gai_status)));
    exit(1);
  }

  int yes{1};

  /* Loop through all results returned by getaddrinfo and bind to the first
   * address that can be bound */
  struct addrinfo *pAddr;
  for (pAddr = addresses; pAddr != nullptr; pAddr = pAddr->ai_next) {
    if ((server_fd = socket(pAddr->ai_family, pAddr->ai_socktype, pAddr->ai_protocol)) == -1) {
      perror("[INFO]: socket error. trying next address...\n");
      continue;
    }

    setnonblocking(server_fd) ;

    /* set socket option to reuse address, preventing "address already in use" error message */
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
      perror("[ERROR]: setsockopt error");
      exit(1);
    }

    if (bind(server_fd, pAddr->ai_addr, pAddr->ai_addrlen) == -1) {
      close(server_fd);
      perror("[INFO]: bind error. trying next address...\n");
      continue;
    }
    break;
  }

  freeaddrinfo(addresses);

  if (pAddr == nullptr) {
      perror("[ERROR]: failed to bind");
      exit(1);
  }

  if (listen(server_fd, 512) == -1) {
    perror("[ERROR]: listen error");
    exit(1);
  }

  if (epoll_man.register_fd(server_fd, EPOLLIN)) {
    perror("[ERROR]: listening socket epoll err");
    exit(1);
  }

  std::cout << "[INFO]: listening for TCP connections at port " << port << "\n";
}

int RedisServer::acceptConn() {
  struct sockaddr_storage client_addr{};
  socklen_t addr_size{sizeof(client_addr)};
  struct sockaddr *p_client_addr;

  p_client_addr = (struct sockaddr *)&client_addr;
  int client_fd = accept(server_fd, p_client_addr, &addr_size);
  if (client_fd == -1) {
    perror("[INFO]: accept error");
    return -2;
  }

  /* register client to epoll set */
  setnonblocking(client_fd);
  if (epoll_man.register_fd(client_fd, EPOLLIN) == -1) {
    return -1;
  }

  /* initialise client buffer */
  if (client_fd >= (int)clients.size()) {
    clients.resize(client_fd + 1);
  }
  clients[client_fd].init(client_fd);

  return 0;
}

std::pair<std::string, int> RedisServer::execSet(Conn& c) {
  auto &args = c.cmd.args;
  if (args.size() != 3) {
    return {"Invalid command args", -1};
  }
  auto &key = args[1];
  auto &value = args[2];
  kvstore.insert({key, value});
  return {"set OK", 0};
}

std::pair<std::string, int> RedisServer::execGet(Conn& c) {
  auto &args = c.cmd.args;
  if (args.size() != 2) {
    return {"Invalid command args", -1};
  }
  auto &key = args[1];
  auto it  = kvstore.find(key);
  if (it == kvstore.end()) {
    return {key + " not found", 1};
  }
  return {it->second, 0};
}

std::pair<std::string, int> RedisServer::execDel(Conn& c) {
  auto &args = c.cmd.args;
  if (args.size() != 2) {
    return {"Invalid command args", -1};
  }
  auto &key = args[1];
  kvstore.erase(key);
  return {"del OK", 0};
}

int RedisServer::execCommand(Conn& c) {
  auto &cmd_type = c.cmd.args[0];

  /* TODO: change to OOP maybe? */
  std::pair<std::string, int> result;
  if (cmd_type == "set") {
    result = execSet(c);
  } else if (cmd_type == "get") {
    result = execGet(c);
  } else if (cmd_type == "del") {
    result = execDel(c);
  } else {
    // error
    result = {"Invalid command", -1};
  }

  auto &[message, return_val] = result;

  /* construct reply */
  uint32_t ret_val = (uint32_t)return_val;
  uint32_t resp_len = (uint32_t)message.size();

  std::memcpy(c.wbuf.data() + c.wbuf_offset, &ret_val, 4);
  c.pending_write_len += 4;

  std::memcpy(c.wbuf.data() + c.wbuf_offset + 4, &resp_len, 4);
  c.pending_write_len += 4;

  if (resp_len > 0) {
    std::memcpy(c.wbuf.data() + c.wbuf_offset + 8, message.data(), resp_len);
    c.pending_write_len += resp_len;
  }

  // std::cout << "[DEBUG]: finished executing command. setting state to STATE_WRITE\n";
  c.state = STATE_WRITE;
  return RedisServer::handleWrite(c);
}

/**
 * Reads message from client and send the response accordingly.
 * 
 * The expected message is of the following format:
 * +------+-----+------+-----+------+--------
 * | nstr | len | msg1 | len | msg2 | more...
 * +------+-----+------+-----+------+--------
 * 
 * Instead of first read()-ing the len and then read()-ing
 * the body, we use one read() syscall to read as many bytes as possible.
 * 
 * If read() returns more than one messages or a partial message,
 * msg_buf_offset and msg_body_offset is used to mark the start
 * of the buffer that can be read/written from/to
 * 
 * @param client_fd the file descriptor of the client
 * @return -1 if there is an error, 0 otherwise
 */
int RedisServer::handleRead(Conn& c) {
  constexpr size_t buffer_size = Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN;
  size_t remaining_size = buffer_size - (c.rbuf_woffset - c.rbuf_roffset);
  ssize_t packet_len = read(c.fd, c.rbuf.data() + c.rbuf_woffset, remaining_size);
  
  /* TODO: check for EINTR ? */

  if (packet_len == -1) {
    perror("[INFO]: read error");
    return -1;
  } else if (packet_len == 0) {
    c.state = STATE_END;
    return 0;
  }

  c.rbuf_woffset += packet_len;

  if (c.rbuf_woffset - c.rbuf_roffset < 4) {
    /* not enough data was read. length of header is 4 bytes
     * return immediately and try to read again on the next epoll cycle */
    Logger::log(Logger::DEBUG, "not enough data read. waiting for next iteration.");
    return 1;
  }

  uint32_t cmd_len;
  while (c.rbuf_woffset - c.rbuf_roffset > 4) {
    Logger::log(Logger::DEBUG, "read buffer offsets:"
        " r = " + std::to_string(c.rbuf_roffset) + " w = " + std::to_string(c.rbuf_woffset));
    if (c.cmd.isEmpty()) {
      /* initialize command struct inside conn */
      Logger::log(Logger::DEBUG, "command struct is empty. initialising");
      uint32_t nstr;
      std::memcpy(&nstr, c.rbuf.data() + c.rbuf_roffset, 4);
      c.rbuf_roffset += 4;
      c.cmd.args_size = nstr;
      continue;
    }

    assert (!c.cmd.isComplete());

    /* read the command arguments */
    std::memcpy(&cmd_len, c.rbuf.data() + c.rbuf_roffset, 4);
    c.rbuf_roffset += 4;

    if (cmd_len > Consts::MAX_MSG_LEN) {
      /* immediately terminate errorneous client for now */
      Logger::log(Logger::INFO, "message len is too long.", false);
      Logger::log(Logger::INFO, " len = " + std::to_string(cmd_len), false);
      Logger::log(Logger::INFO, " fd = " + std::to_string(c.fd));
      c.state = STATE_END;
      return -1;
    }

    if (cmd_len > (c.rbuf_woffset - c.rbuf_roffset)) {
      /* partial read
      * shift the data to the start of the buffer
      * and try reading again on the next epoll call */
      Logger::log(Logger::DEBUG, "partial read. shifting buffer");
      c.rbuf_roffset -= 4; /* restore len header */
      std::memmove(c.rbuf.data(), c.rbuf.data() + c.rbuf_roffset, (c.rbuf_woffset - c.rbuf_roffset));
      c.rbuf_woffset -= c.rbuf_roffset;
      c.rbuf_roffset = 0;
      return 0;
    }

    // char tmp = c.rbuf[c.rbuf_roffset + cmd_len];
    // c.rbuf[c.rbuf_roffset + cmd_len] = '\0';
    // std::cout << "[INFO]: client says " << c.rbuf.data() + c.rbuf_roffset << "\n";
    // c.rbuf[c.rbuf_roffset + cmd_len] = tmp; // restore 

    c.cmd.addArgs(std::string((char *)(c.rbuf.data() + c.rbuf_roffset), cmd_len));
    c.rbuf_roffset += cmd_len;

    if (c.cmd.isComplete()) {
      /* execute command */

      Logger::log(Logger::DEBUG, "executing command");
      Logger::log(Logger::DEBUG, "cmd: args");
      Logger::log(Logger::DEBUG, c.cmd.args.begin(), c.cmd.args.end());

      int err = execCommand(c);
      if (err == -1) {
        Logger::log(Logger::DEBUG, "error while executing command (write err). setting state to STATE_END");
        c.state = STATE_END;
        return -1;
      }

      /* reset cmd struct for next command */
      c.cmd.reset();
    }
  }

  /* reset read buffer offset
   * when there is no unconsumed data in the read buffer
   * i.e. read offset == write offset, 
   * safely reset these offsets to 0
   * 
   * otherwise, move the data forward
   * and decrement write offset */
  if (c.rbuf_roffset != c.rbuf_woffset) {
    std::memmove(c.rbuf.data(), c.rbuf.data() + c.rbuf_roffset, (c.rbuf_woffset - c.rbuf_roffset));
    c.rbuf_woffset -= c.rbuf_roffset;
  } else {
    c.rbuf_woffset = 0;
  }
  c.rbuf_roffset = 0;
  return 0;
}

int RedisServer::handleWrite(Conn& c) {
  ssize_t rv = write(c.fd, c.wbuf.data() + c.wbuf_offset, c.pending_write_len);
  if (rv == -1) {
    Logger::log(Logger::DEBUG, "write err. setting state to STATE_END");
    c.state = STATE_END;
    return -1;
  }

  c.wbuf_offset += rv;
  c.pending_write_len -= rv;

  if (c.pending_write_len == 0) {
    Logger::log(Logger::DEBUG, "finished writing. setting state to STATE_READ");
    /* finished writing */
    c.state = STATE_READ;
    c.wbuf_offset = 0;
    c.pending_write_len = 0;
  } 

  return 0;
}

void RedisServer::runServer() {
  std::vector<struct epoll_event> events(Consts::MAX_EPOLL_EVENTS);
  for (;;) {
    int nfds = epoll_man.poll(events);
    for (int i = 0; i < nfds; ++i) {
      int client_fd = events[i].data.fd;
      int err = 0;

      if (client_fd == server_fd) {
        err = RedisServer::acceptConn();
      } else if ((events[i].events & EPOLLIN) != 0) {
        err = RedisServer::handleRead(clients[client_fd]);
        if (clients[client_fd].state == STATE_WRITE) {
          epoll_man.mod_fd(client_fd, EPOLLOUT);
          Logger::log(Logger::DEBUG, "mod fd: EPOLLIN -> EPOLLOUT");
        }
      } else if ((events[i].events & EPOLLOUT) != 0) {
        err = RedisServer::handleWrite(clients[client_fd]);
        if (clients[client_fd].state == STATE_READ) {
          epoll_man.mod_fd(client_fd, EPOLLIN);
          Logger::log(Logger::DEBUG, "mod fd: EPOLLOUT -> EPOLLIN");
        }
      }

      if (err == -1 || clients[client_fd].state == STATE_END) {
        epoll_man.delete_fd(client_fd);
        clients[client_fd].reset();
      }
    }
  }
}

int main() {
  RedisServer s{};
  s.startTcpServer(6969);
  s.runServer();
  return 0;
}
