#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils.hpp"
#include "conn.hpp"
#include "logger.hpp"
#include <vector>
#include <string>
#include <cstdint>

const size_t k_max_msg = 4096;

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Helper to send command in server's expected protocol
static int32_t send_command(int fd, const std::vector<std::string>& args) {
    uint32_t n_args = args.size();
    std::vector<uint8_t> buf;
    // Add number of arguments
    for (size_t i = 0; i < 4; ++i) buf.push_back((n_args >> (i * 8)) & 0xFF);
    // For each argument, add length and data
    for (const auto& arg : args) {
        uint32_t len = arg.size();
        for (size_t i = 0; i < 4; ++i) buf.push_back((len >> (i * 8)) & 0xFF);
        buf.insert(buf.end(), arg.begin(), arg.end());
    }
    ssize_t total_sent = 0;
    while (total_sent < (ssize_t)buf.size()) {
        ssize_t sent = send(fd, buf.data() + total_sent, buf.size() - total_sent, 0);
        if (sent < 0) return -1;
        total_sent += sent;
    }
    return 0;
}

static int32_t receive_reply(int fd) {
    char rbuf[4 + 4 + k_max_msg + 1];
    uint32_t retcode, len;

    if (recv(fd, &retcode, sizeof(retcode), 0) <= 0 ||
        recv(fd, &len, sizeof(len), 0) <= 0) {
        return -1;
    }

    if (recv(fd, rbuf, len, 0) <= 0) {
        return -1;
    }

    rbuf[len] = '\0';
    std::cout << "REPLY: " << rbuf << std::endl;
    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6969);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        die("connect()");
    }

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        std::vector<std::string> args;
        size_t pos = 0, next;
        while ((next = line.find(' ', pos)) != std::string::npos) {
            args.push_back(line.substr(pos, next - pos));
            pos = next + 1;
        }
        args.push_back(line.substr(pos));
        if (args[0] == "exit" || args[0] == "quit") break;
        if (send_command(fd, args) != 0) {
            die("send_command()");
        }
        if (receive_reply(fd) != 0) {
            die("receive_reply()");
        }
    }
    close(fd);
    return 0;
}