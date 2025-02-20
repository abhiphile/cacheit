#include <cstdint>
#include <stddef.h>
#include <vector>
#include "command.h"
#include "utils.h"

#pragma once

enum ConnState {
  STATE_READ,
  STATE_WRITE,
  STATE_END
};

/**
 * Conn struct: a structure that represents a connection
 * 
 * This struct contains the following fields:
 *   int fd;                      : the file descriptor of the connection
 *   ConnState state{};           : state of the connection -> whether the server needs to read from the socket, or send a reply.
 *   size_t rbuf_roffset{};       : read buffer's read offset (used when consuming data on buffer)
 *   size_t rbuf_woffset{};       : read bufer's write offset (used when reading from network to read buffer)
 *   size_t wbuf_offset{};        : write buffer's write offset (used when writing data to the network)
 *   size_t pending_write_len{};  : amount of data still pending in the write buffer
 *   char* rbuf;                  : pointer to the read buffer
 *   char* wbuf;                  : pointer to the write buffer
 * 
 */
struct Conn {
  Conn()
  : Conn(-1)
  {}

  Conn(int fd)
  : fd{fd},
    state{STATE_READ},
    rbuf(Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN),
    wbuf(Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN)
  {}

  // Conn(const Conn& other) = delete;
  // Conn& operator=(const Conn& other) = delete;

  // Conn(Conn&& other) = default;
  // Conn& operator=(Conn&& other) = default;

  void reset() {
    fd = -1;
    state = STATE_READ;
    rbuf_roffset = 0;
    rbuf_woffset = 0;
    wbuf_offset = 0;
    pending_write_len = 0;
    cmd.reset();
  }

  void init(int fd) {
    reset();
    this->fd = fd;
  }

  int fd;
  ConnState state{};
  size_t rbuf_roffset{};
  size_t rbuf_woffset{};
  size_t wbuf_offset{};
  size_t pending_write_len{};
  Command cmd{};
  std::vector<char> rbuf;
  std::vector<char> wbuf;
};