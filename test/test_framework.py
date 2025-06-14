import socket
import struct
import sys
import time
from test_utils import print_separator

HOST = '127.0.0.1'
PORT = 6969

def send_command(sock, args):
    buf = b''
    buf += struct.pack('<I', len(args))
    for arg in args:
        arg_bytes = arg.encode()
        buf += struct.pack('<I', len(arg_bytes))
        buf += arg_bytes
    sock.sendall(buf)
    # Receive reply
    retcode = struct.unpack('<I', sock.recv(4))[0]
    length = struct.unpack('<I', sock.recv(4))[0]
    data = b''
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            break
        data += chunk
    return retcode, data.decode()

def run_test_case(cmd, expected_reply):
    with socket.create_connection((HOST, PORT)) as sock:
        ret, reply = send_command(sock, cmd)
        assert reply == expected_reply, f"Expected: {expected_reply}, Got: {reply}"
        print(f"PASS: {' '.join(cmd)} => {reply}")

def main():
    # Wait for server to be ready
    for _ in range(10):
        try:
            with socket.create_connection((HOST, PORT)):
                break
        except Exception:
            time.sleep(0.5)
    print_separator("Basic string tests")
    run_test_case(["set", "foo", "bar"], "set OK")
    run_test_case(["get", "foo"], "bar")
    run_test_case(["get", "baz"], "baz not found")
    print_separator("List tests")
    run_test_case(["lpush", "mylist", "42"], "pushed 42 to mylist")
    run_test_case(["lrange", "mylist", "0", "-1"], "42")
    print_separator("Set tests")
    run_test_case(["sadd", "myset", "a", "b"], "added 2")
    run_test_case(["smembers", "myset"], "a b")
    print_separator("Hash tests")
    run_test_case(["hset", "myhash", "field1", "val1"], "OK")
    run_test_case(["hget", "myhash", "field1"], "val1")
    run_test_case(["hdel", "myhash", "field1"], "removed 1")
    run_test_case(["hget", "myhash", "field1"], "not found")
    print_separator("Sorted set tests")
    run_test_case(["zadd", "myzset", "1.0", "foo"], "OK")
    run_test_case(["zrange", "myzset", "0", "-1"], "foo")
    run_test_case(["zadd", "myzset", "2.0", "bar"], "OK")
    run_test_case(["zrange", "myzset", "0", "-1"], "foo bar")
    run_test_case(["zrem", "myzset", "foo"], "removed 1")
    run_test_case(["zrange", "myzset", "0", "-1"], "bar")
    print_separator("Bitmap tests")
    run_test_case(["setbit", "mybits", "5", "1"], "OK")
    run_test_case(["getbit", "mybits", "5"], "1")
    run_test_case(["bitcount", "mybits"], "1")
    print_separator("All tests passed!")

if __name__ == "__main__":
    main()
