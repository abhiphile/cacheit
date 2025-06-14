// filepath: /cacheit/cacheit/include/conn.hpp
#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "command.hpp"
#include "utils.hpp"

enum class ConnState {
    STATE_READ,
    STATE_WRITE,
    STATE_END
};

struct Conn {
    Conn();
    Conn(int fd);
    void reset();
    void init(int fd);

    int fd;
    ConnState state{};
    size_t rbuf_roffset{};
    size_t rbuf_woffset{};
    size_t wbuf_offset{};
    size_t pending_write_len{};
    Command cmd;
    std::vector<uint8_t> rbuf;
    std::vector<uint8_t> wbuf;
};