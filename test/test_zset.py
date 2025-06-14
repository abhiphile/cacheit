from testlib import send_command, wait_for_server, color_text
from test_utils import print_separator
import socket

def test_zset():
    wait_for_server()
    print_separator("Sorted Set tests")
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["zadd", "myzset", "1.0", "foo"])
        assert reply == "OK"
        print(color_text("PASS: zadd myzset 1.0 foo", "green"))
        ret, reply = send_command(sock, ["zadd", "myzset", "2.0", "bar"])
        assert reply == "OK"
        print(color_text("PASS: zadd myzset 2.0 bar", "green"))
        ret, reply = send_command(sock, ["zrange", "myzset", "0", "-1"])
        assert set(reply.split()) == {"foo", "bar"}
        print(color_text("PASS: zrange myzset 0 -1", "green"))
        ret, reply = send_command(sock, ["zrem", "myzset", "foo"])
        assert reply == "OK"
        print(color_text("PASS: zrem myzset foo", "green"))
        ret, reply = send_command(sock, ["zrange", "myzset", "0", "-1"])
        assert set(reply.split()) == {"bar"}
        print(color_text("PASS: zrange myzset 0 -1 (after remove)", "green"))
    print(color_text("Sorted Set tests passed!", "cyan"))

if __name__ == "__main__":
    test_zset()
