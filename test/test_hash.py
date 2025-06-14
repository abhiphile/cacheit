from testlib import send_command, wait_for_server, color_text
from test_utils import print_separator
import socket

def test_hash():
    wait_for_server()
    print_separator("Hash tests")
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["hset", "myhash", "field1", "val1"])
        assert reply == "OK"
        print(color_text("PASS: hset myhash field1 val1", "green"))
        ret, reply = send_command(sock, ["hget", "myhash", "field1"])
        assert reply == "val1"
        print(color_text("PASS: hget myhash field1", "green"))
        ret, reply = send_command(sock, ["hdel", "myhash", "field1"])
        assert reply == "OK"
        print(color_text("PASS: hdel myhash field1", "green"))
        ret, reply = send_command(sock, ["hget", "myhash", "field1"])
        assert reply == "not found"
        print(color_text("PASS: hget myhash field1 (after delete)", "green"))
    print(color_text("Hash tests passed!", "cyan"))

if __name__ == "__main__":
    test_hash()
