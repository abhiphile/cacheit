#include "conn.hpp"
#include <cstring>
#include <vector>
#include "utils.hpp"

Conn::Conn() : Conn(-1) {}

Conn::Conn(int fd)
: fd{fd},
  state{ConnState::STATE_READ},
  rbuf(Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN),
  wbuf(Consts::SIZE_HEADER_LEN + Consts::MAX_MSG_LEN) {}

void Conn::reset() {
    fd = -1;
    state = ConnState::STATE_READ;
    rbuf_roffset = 0;
    rbuf_woffset = 0;
    wbuf_offset = 0;
    pending_write_len = 0;
    cmd.reset();
}

void Conn::init(int fd) {
    reset();
    this->fd = fd;
}