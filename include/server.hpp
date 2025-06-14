// filepath: /cacheit/cacheit/include/server.hpp
#pragma once

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
#include <unordered_set>
#include <map>
#include <deque>
#include <bitset>
#include <vector>
#include "command.hpp"
#include "conn.hpp"
#include "epoll_manager.hpp"
#include "utils.hpp"

class RedisServer {
public:
    RedisServer();
    void startTcpServer(int port);
    void runServer();
    int acceptConn();
    std::pair<std::string, int> execSet(Conn& c);
    std::pair<std::string, int> execGet(Conn& c);
    std::pair<std::string, int> execDel(Conn& c);
    int execCommand(Conn& c);
    int handleRead(Conn& c);
    int handleWrite(Conn& c);
    std::pair<std::string, int> execLpush(Conn& c);
    std::pair<std::string, int> execLrange(Conn& c);
    std::pair<std::string, int> execLrem(Conn& c);
    // Set (SADD, SREM, SMEMBERS)
    std::unordered_map<std::string, std::unordered_set<std::string>> setstore;
    // Hash (HSET, HGET, HDEL, HGETALL)
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hashmapstore;
    // Sorted Set (ZADD, ZRANGE, ZREM)
    std::unordered_map<std::string, std::map<double, std::string>> zsetstore;
    // Bitmap (SETBIT, GETBIT, BITCOUNT)
    std::unordered_map<std::string, std::vector<bool>> bitmapstore;

private:
    int server_fd;
    EpollManager epoll_man{};
    std::unordered_map<std::string, std::string> kvstore;
    std::vector<Conn> clients;
    // Set
    std::pair<std::string, int> execSadd(Conn& c);
    std::pair<std::string, int> execSrem(Conn& c);
    std::pair<std::string, int> execSmembers(Conn& c);
    // Hash
    std::pair<std::string, int> execHset(Conn& c);
    std::pair<std::string, int> execHget(Conn& c);
    std::pair<std::string, int> execHdel(Conn& c);
    std::pair<std::string, int> execHgetall(Conn& c);
    // Sorted Set
    std::pair<std::string, int> execZadd(Conn& c);
    std::pair<std::string, int> execZrange(Conn& c);
    std::pair<std::string, int> execZrem(Conn& c);
    // Bitmap
    std::pair<std::string, int> execSetbit(Conn& c);
    std::pair<std::string, int> execGetbit(Conn& c);
    std::pair<std::string, int> execBitcount(Conn& c);
};