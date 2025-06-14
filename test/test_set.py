from testlib import send_command, wait_for_server, color_text
from test_utils import print_separator
import socket

def test_set():
    wait_for_server()
    print_separator("Set tests")
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["sadd", "myset", "a", "b", "c"])
        assert reply == "OK"
        print(color_text("PASS: sadd myset a b c", "green"))
        ret, reply = send_command(sock, ["smembers", "myset"])
        assert set(reply.split()) == {"a", "b", "c"}
        print(color_text("PASS: smembers myset", "green"))
        ret, reply = send_command(sock, ["srem", "myset", "b"])
        assert reply == "OK"
        print(color_text("PASS: srem myset b", "green"))
        ret, reply = send_command(sock, ["smembers", "myset"])
        assert set(reply.split()) == {"a", "c"}
        print(color_text("PASS: smembers myset (after remove)", "green"))
    print(color_text("Set tests passed!", "cyan"))

if __name__ == "__main__":
    test_set()
