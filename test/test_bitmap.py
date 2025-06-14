from testlib import send_command, wait_for_server, color_text
from test_utils import print_separator
import socket

def test_bitmap():
    wait_for_server()
    print_separator("Bitmap tests")
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["setbit", "mybits", "5", "1"])
        assert reply == "OK"
        print(color_text("PASS: setbit mybits 5 1", "green"))
        ret, reply = send_command(sock, ["getbit", "mybits", "5"])
        assert reply == "1"
        print(color_text("PASS: getbit mybits 5", "green"))
        ret, reply = send_command(sock, ["bitcount", "mybits"])
        assert reply == "1"
        print(color_text("PASS: bitcount mybits", "green"))
        ret, reply = send_command(sock, ["setbit", "mybits", "5", "0"])
        assert reply == "OK"
        print(color_text("PASS: setbit mybits 5 0", "green"))
        ret, reply = send_command(sock, ["getbit", "mybits", "5"])
        assert reply == "0"
        print(color_text("PASS: getbit mybits 5 (after clear)", "green"))
    print(color_text("Bitmap tests passed!", "cyan"))

if __name__ == "__main__":
    test_bitmap()
