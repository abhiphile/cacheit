#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

const size_t k_max_msg = 4096;

static void msg(const char *msg) {
  fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static int32_t read_full(int fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);
    if (rv <= 0) {
      return -1;  // error, or unexpected EOF
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);
    if (rv <= 0) {
      return -1;  // error
    }
    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

int32_t prepare_set_cmd(const char *key, const char *val, char *wbuf) {
  const uint32_t nargs = 3;
  const char *SET = "set";

  uint32_t set_len = (uint32_t)strlen(SET);
  uint32_t key_len = (uint32_t)strlen(key);
  uint32_t val_len = (uint32_t)strlen(val);

  size_t offset = 0;

  memcpy(wbuf + offset, &nargs, 4); 
  offset += 4;

  memcpy(wbuf + offset, &set_len, 4);
  offset += 4;
  memcpy(wbuf + offset, SET, set_len);
  offset += set_len;

  memcpy(wbuf + offset, &key_len, 4);
  offset += 4;
  memcpy(wbuf + offset, key, key_len);
  offset += key_len;

  memcpy(wbuf + offset, &val_len, 4);
  offset += 4;
  memcpy(wbuf + offset, val, val_len);
  offset += val_len;

  size_t total_message_len = 4 + (4 + 3) + (4 + key_len) + (4 + val_len);
  assert (offset == total_message_len);
  printf("REQUEST : set %s %s\n", key, val);
  return total_message_len;
}

int32_t prepare_get_cmd(const char *key, char *wbuf) {
  const uint32_t nargs = 2;
  const char *GET = "get";

  uint32_t get_len = (uint32_t)strlen(GET);
  uint32_t key_len = (uint32_t)strlen(key);

  size_t offset = 0;

  memcpy(wbuf + offset, &nargs, 4); 
  offset += 4;

  memcpy(wbuf + offset, &get_len, 4);
  offset += 4;
  memcpy(wbuf + offset, GET, get_len);
  offset += get_len;

  memcpy(wbuf + offset, &key_len, 4);
  offset += 4;
  memcpy(wbuf + offset, key, key_len);
  offset += key_len;

  size_t total_message_len = 4 + (4 + 3) + (4 + key_len);
  assert (offset == total_message_len);
  printf("REQUEST : get %s\n", key);
  return total_message_len;
}

int32_t prepare_del_cmd(const char *key, char *wbuf) {
  const uint32_t nargs = 2;
  const char *DEL = "del";

  uint32_t del_len = (uint32_t)strlen(DEL);
  uint32_t key_len = (uint32_t)strlen(key);

  size_t offset = 0;

  memcpy(wbuf + offset, &nargs, 4); 
  offset += 4;

  memcpy(wbuf + offset, &del_len, 4);
  offset += 4;
  memcpy(wbuf + offset, DEL, del_len);
  offset += del_len;

  memcpy(wbuf + offset, &key_len, 4);
  offset += 4;
  memcpy(wbuf + offset, key, key_len);
  offset += key_len;

  size_t total_message_len = 4 + (4 + 3) + (4 + key_len);
  assert (offset == total_message_len);
  printf("REQUEST : del %s\n", key);
  return total_message_len;
}

int32_t prepare_malformed(char *wbuf) {
  const uint32_t nargs = 5;
  const char *CMD = "abcdefg";

  uint32_t cmd_len = (uint32_t)strlen(CMD);

  size_t offset = 0;

  memcpy(wbuf + offset, &nargs, 4); 
  offset += 4;

  memcpy(wbuf + offset, &cmd_len, 4);
  offset += 4;
  memcpy(wbuf + offset, CMD, cmd_len);
  offset += cmd_len;

  memset(wbuf + offset, 0, 2048);
  offset += 2048;

  printf("REQUEST : %s\n", CMD);
  return offset;
}

int32_t send_command(int fd, const char *buf, size_t len) {
  return write_all(fd, buf, len);
}

int32_t send_command_fragmented(int fd, const char *buf, size_t len) {
  size_t half = len / 2;
  int32_t err = write_all(fd, buf, half);
  if (err) return err;
  sleep(3);
  return write_all(fd, buf + half, len - half);
}

int32_t receive_reply(int fd) {

  /*         4 bytes return code
   *         4 bytes message len
   * k_max_msg bytes message body */

  char rbuf[4 + 4 + k_max_msg + 1];
  errno = 0;

  /* read ret code*/
  int32_t err = read_full(fd, rbuf, 4);
  if (err) {
    msg("read() error");
    return err;
  }
  uint32_t retcode;
  memcpy(&retcode, rbuf, 4);

  /* read message len */
  err = read_full(fd, rbuf, 4);
  if (err) {
    msg("read() error");
    return err;
  }
  uint32_t len;
  memcpy(&len, rbuf, 4);
  
  /* assume server's reply length is always valid */

  /* read reply body */
  err = read_full(fd, rbuf, len);
  if (err) {
    msg("read() error");
    return err;
  }

  rbuf[len] = '\0';
  printf("REPLY   : %s\n", rbuf);
  return 0;
}

int runTests(int fd) {
  char buf[2 * k_max_msg];
  size_t message_len;
  /* test1: send normal request */
  memset(buf, 0, 2 * k_max_msg);
  message_len = prepare_set_cmd("hello", "world", buf);
  if (send_command(fd, buf, message_len) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;

  /* test2: send fragmented command (partial read on server side) */
  memset(buf, 0, 2 * k_max_msg);
  message_len = prepare_set_cmd("key", "value", buf);
  if (send_command_fragmented(fd, buf, message_len) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;

  /* test3: send multiple commands using one write() call */
  memset(buf, 0, 2 * k_max_msg);
  message_len = prepare_get_cmd("hello", buf);
  message_len += prepare_get_cmd("nokey", buf + message_len);
  if (send_command_fragmented(fd, buf, message_len) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;

  /* test4: send multiple commands using one write() call, fragmented */
  memset(buf, 0, 2 * k_max_msg);
  message_len = prepare_del_cmd("hello000", buf);
  message_len += prepare_get_cmd("hello", buf + message_len);
  if (send_command_fragmented(fd, buf, message_len) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;

  /* test5: send malformed commands */
  message_len = prepare_malformed(buf);
  if (send_command(fd, buf, message_len) != 0) return -1;
  if (receive_reply(fd) != 0) return -1;
  return 0;
}

int main() {
  /* disable printf buffering */
  setbuf(stdout, NULL);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(6969);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("can't connect to server");
  }

  int err = runTests(fd);

  close(fd);

  return err;
}