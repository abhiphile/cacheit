#include "server.hpp"
#include "epoll_manager.hpp"
#include "conn.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "datastructures/linked_list.hpp"
#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <sstream>

RedisServer::RedisServer() 
  : clients(10) {
}

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

  struct addrinfo *pAddr;
  for (pAddr = addresses; pAddr != nullptr; pAddr = pAddr->ai_next) {
    if ((server_fd = socket(pAddr->ai_family, pAddr->ai_socktype, pAddr->ai_protocol)) == -1) {
      perror("[INFO]: socket error. trying next address...\n");
      continue;
    }

    setnonblocking(server_fd);

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

  setnonblocking(client_fd);
  if (epoll_man.register_fd(client_fd, EPOLLIN) == -1) {
    return -1;
  }

  if (client_fd >= (int)clients.size()) {
    clients.resize(client_fd + 1);
  }
  clients[client_fd].init(client_fd);

  return 0;
}

std::pair<std::string, int> RedisServer::execSet(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) {
    return {"Invalid command args", -1};
  }
  const auto &key = args[1];
  const auto &value = args[2];
  kvstore.insert({key, value});
  Logger::log(Logger::INFO, "SET key: '" + key + "' value: '" + value + "'");
  return {"set OK", 0};
}

std::pair<std::string, int> RedisServer::execGet(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 2) {
    return {"Invalid command args", -1};
  }
  const auto &key = args[1];
  auto it  = kvstore.find(key);
  if (it == kvstore.end()) {
    Logger::log(Logger::INFO, "GET key: '" + key + "' not found");
    return {key + " not found", 1};
  }
  Logger::log(Logger::INFO, "GET key: '" + key + "' value: '" + it->second + "'");
  return {it->second, 0};
}

std::pair<std::string, int> RedisServer::execDel(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 2) {
    return {"Invalid command args", -1};
  }
  const auto &key = args[1];
  kvstore.erase(key);
  return {"del OK", 0};
}

// Add a map for named lists like Redis
std::unordered_map<std::string, LinkedList> liststore;

std::pair<std::string, int> RedisServer::execLpush(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) {
    return {"Invalid command args", -1};
  }
  const auto &listname = args[1];
  int value = std::stoi(args[2]);
  liststore[listname].append(value);
  return {"pushed " + args[2] + " to " + listname, 0};
}

std::pair<std::string, int> RedisServer::execLrange(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) {
    return {"Invalid command args", -1};
  }
  const auto &listname = args[1];
  int start = std::stoi(args[2]);
  int end = std::stoi(args[3]);
  auto it = liststore.find(listname);
  if (it == liststore.end()) return {"list not found", 1};
  LinkedList &list = it->second;
  std::ostringstream oss;
  LinkedList::Node* node = list.getHead();
  int idx = 0;
  while (node) {
    if (idx >= start && (end == -1 || idx <= end)) oss << node->data << " ";
    node = node->next;
    ++idx;
    if (end != -1 && idx > end) break;
  }
  std::string out = oss.str();
  if (!out.empty() && out.back() == ' ') out.pop_back();
  return {out, 0};
}

std::pair<std::string, int> RedisServer::execLrem(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) {
    return {"Invalid command args", -1};
  }
  const auto &listname = args[1];
  int count = std::stoi(args[2]);
  int value = std::stoi(args[3]);
  auto it = liststore.find(listname);
  if (it == liststore.end()) return {"list not found", 1};
  int removed = 0;
  for (int i = 0; i < count; ++i) {
    size_t before = it->second.getSize();
    it->second.remove(value);
    if (it->second.getSize() < before) ++removed;
    else break;
  }
  std::ostringstream oss;
  oss << "removed " << removed << " of " << value;
  return {oss.str(), 0};
}

// SET COMMANDS
std::pair<std::string, int> RedisServer::execSadd(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() < 3) return {"Invalid command args", -1};
  const auto &setname = args[1];
  int added = 0;
  for (size_t i = 2; i < args.size(); ++i) {
    added += setstore[setname].insert(args[i]).second ? 1 : 0;
  }
  return {"added " + std::to_string(added), 0};
}
std::pair<std::string, int> RedisServer::execSrem(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() < 3) return {"Invalid command args", -1};
  const auto &setname = args[1];
  int removed = 0;
  for (size_t i = 2; i < args.size(); ++i) {
    removed += setstore[setname].erase(args[i]);
  }
  return {"removed " + std::to_string(removed), 0};
}
std::pair<std::string, int> RedisServer::execSmembers(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 2) return {"Invalid command args", -1};
  const auto &setname = args[1];
  auto it = setstore.find(setname);
  if (it == setstore.end()) return {"set not found", 1};
  std::string out;
  for (const auto &v : it->second) out += v + " ";
  if (!out.empty()) out.pop_back();
  return {out, 0};
}
// HASH COMMANDS
std::pair<std::string, int> RedisServer::execHset(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) return {"Invalid command args", -1};
  hashmapstore[args[1]][args[2]] = args[3];
  return {"OK", 0};
}
std::pair<std::string, int> RedisServer::execHget(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) return {"Invalid command args", -1};
  auto it = hashmapstore.find(args[1]);
  if (it == hashmapstore.end()) return {"not found", 1};
  auto jt = it->second.find(args[2]);
  if (jt == it->second.end()) return {"not found", 1};
  return {jt->second, 0};
}
std::pair<std::string, int> RedisServer::execHdel(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) return {"Invalid command args", -1};
  auto it = hashmapstore.find(args[1]);
  if (it == hashmapstore.end()) return {"not found", 1};
  int removed = it->second.erase(args[2]);
  return {"removed " + std::to_string(removed), 0};
}
std::pair<std::string, int> RedisServer::execHgetall(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 2) return {"Invalid command args", -1};
  auto it = hashmapstore.find(args[1]);
  if (it == hashmapstore.end()) return {"not found", 1};
  std::string out;
  for (const auto &kv : it->second) out += kv.first + ":" + kv.second + " ";
  if (!out.empty()) out.pop_back();
  return {out, 0};
}
// ZSET COMMANDS
std::pair<std::string, int> RedisServer::execZadd(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) return {"Invalid command args", -1};
  double score = std::stod(args[2]);
  zsetstore[args[1]][score] = args[3];
  return {"OK", 0};
}
std::pair<std::string, int> RedisServer::execZrange(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) return {"Invalid command args", -1};
  int start = std::stoi(args[2]);
  int end = std::stoi(args[3]);
  auto it = zsetstore.find(args[1]);
  if (it == zsetstore.end()) return {"not found", 1};
  std::string out;
  int idx = 0;
  for (const auto &kv : it->second) {
    if (idx >= start && (end == -1 || idx <= end)) out += kv.second + " ";
    ++idx;
    if (end != -1 && idx > end) break;
  }
  if (!out.empty()) out.pop_back();
  return {out, 0};
}
std::pair<std::string, int> RedisServer::execZrem(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) return {"Invalid command args", -1};
  auto it = zsetstore.find(args[1]);
  if (it == zsetstore.end()) return {"not found", 1};
  int removed = 0;
  for (auto iter = it->second.begin(); iter != it->second.end(); ) {
    if (iter->second == args[2]) { iter = it->second.erase(iter); ++removed; }
    else ++iter;
  }
  return {"removed " + std::to_string(removed), 0};
}
// BITMAP COMMANDS
std::pair<std::string, int> RedisServer::execSetbit(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 4) return {"Invalid command args", -1};
  auto &bits = bitmapstore[args[1]];
  size_t idx = std::stoul(args[2]);
  if (bits.size() <= idx) bits.resize(idx+1);
  bits[idx] = (args[3] == "1");
  return {"OK", 0};
}
std::pair<std::string, int> RedisServer::execGetbit(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 3) return {"Invalid command args", -1};
  auto it = bitmapstore.find(args[1]);
  if (it == bitmapstore.end()) return {"0", 0};
  size_t idx = std::stoul(args[2]);
  if (it->second.size() <= idx) return {"0", 0};
  return {it->second[idx] ? "1" : "0", 0};
}
std::pair<std::string, int> RedisServer::execBitcount(Conn& c) {
  const auto &args = c.cmd.getArgs();
  if (args.size() != 2) return {"Invalid command args", -1};
  auto it = bitmapstore.find(args[1]);
  if (it == bitmapstore.end()) return {"0", 0};
  int count = 0;
  for (bool b : it->second) if (b) ++count;
  return {std::to_string(count), 0};
}

int RedisServer::execCommand(Conn& c) {
  const auto &args = c.cmd.getArgs();
  const auto &cmd_type = args[0];

  std::pair<std::string, int> result;
  if (cmd_type == "set") {
    result = execSet(c);
  } else if (cmd_type == "get") {
    result = execGet(c);
  } else if (cmd_type == "del") {
    result = execDel(c);
  } else if (cmd_type == "lpush") {
    result = execLpush(c);
  } else if (cmd_type == "lrange") {
    result = execLrange(c);
  } else if (cmd_type == "lrem") {
    result = execLrem(c);
  } else if (cmd_type == "sadd") {
    result = execSadd(c);
  } else if (cmd_type == "srem") {
    result = execSrem(c);
  } else if (cmd_type == "smembers") {
    result = execSmembers(c);
  } else if (cmd_type == "hset") {
    result = execHset(c);
  } else if (cmd_type == "hget") {
    result = execHget(c);
  } else if (cmd_type == "hdel") {
    result = execHdel(c);
  } else if (cmd_type == "hgetall") {
    result = execHgetall(c);
  } else if (cmd_type == "zadd") {
    result = execZadd(c);
  } else if (cmd_type == "zrange") {
    result = execZrange(c);
  } else if (cmd_type == "zrem") {
    result = execZrem(c);
  } else if (cmd_type == "setbit") {
    result = execSetbit(c);
  } else if (cmd_type == "getbit") {
    result = execGetbit(c);
  } else if (cmd_type == "bitcount") {
    result = execBitcount(c);
  } else {
    result = {"Invalid command", -1};
  }

  auto &[message, return_val] = result;

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

  c.state = ConnState::STATE_WRITE;
  return RedisServer::handleWrite(c);
}

int RedisServer::handleRead(Conn& c) {
  constexpr size_t buffer_size = Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN;
  size_t remaining_size = buffer_size - (c.rbuf_woffset - c.rbuf_roffset);
  ssize_t packet_len = read(c.fd, c.rbuf.data() + c.rbuf_woffset, remaining_size);
  
  if (packet_len == -1) {
    perror("[INFO]: read error");
    return -1;
  } else if (packet_len == 0) {
    c.state = ConnState::STATE_END;
    return 0;
  }

  c.rbuf_woffset += packet_len;

  if (c.rbuf_woffset - c.rbuf_roffset < 4) {
    return 1;
  }

  uint32_t cmd_len;
  while (c.rbuf_woffset - c.rbuf_roffset > 4) {
    if (c.cmd.isEmpty()) {
      uint32_t nstr;
      std::memcpy(&nstr, c.rbuf.data() + c.rbuf_roffset, 4);
      c.rbuf_roffset += 4;
      c.cmd = Command(nstr); // Properly set the expected number of arguments
      continue;
    }

    assert (!c.cmd.isComplete());

    std::memcpy(&cmd_len, c.rbuf.data() + c.rbuf_roffset, 4);
    c.rbuf_roffset += 4;

    if (cmd_len > Consts::MAX_MSG_LEN) {
      c.state = ConnState::STATE_END;
      return -1;
    }

    if (cmd_len > (c.rbuf_woffset - c.rbuf_roffset)) {
      c.rbuf_roffset -= 4;
      std::memmove(c.rbuf.data(), c.rbuf.data() + c.rbuf_roffset, (c.rbuf_woffset - c.rbuf_roffset));
      c.rbuf_woffset -= c.rbuf_roffset;
      return 0;
    }

    c.cmd.addArgs(std::string((char *)(c.rbuf.data() + c.rbuf_roffset), cmd_len));
    c.rbuf_roffset += cmd_len;

    if (c.cmd.isComplete()) {
      int err = execCommand(c);
      if (err == -1) {
        c.state = ConnState::STATE_END;
        return -1;
      }
      c.cmd.reset();
    }
  }

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
    c.state = ConnState::STATE_END;
    return -1;
  }

  c.wbuf_offset += rv;
  c.pending_write_len -= rv;

  if (c.pending_write_len == 0) {
    c.state = ConnState::STATE_READ;
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
        if (clients[client_fd].state == ConnState::STATE_WRITE) {
          epoll_man.mod_fd(client_fd, EPOLLOUT);
        }
      } else if ((events[i].events & EPOLLOUT) != 0) {
        err = RedisServer::handleWrite(clients[client_fd]);
        if (clients[client_fd].state == ConnState::STATE_READ) {
          epoll_man.mod_fd(client_fd, EPOLLIN);
        }
      }

      if (err == -1 || clients[client_fd].state == ConnState::STATE_END) {
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