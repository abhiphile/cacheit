from testlib import send_command, wait_for_server, color_text
from test_utils import print_separator
import socket

def test_string():
    wait_for_server()
    print_separator("String (key-value) tests")
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["set", "foo", "bar"])
        assert reply == "OK", f"Expected 'OK', got '{reply}'"
        print(color_text("PASS: set foo bar", "green"))
        ret, reply = send_command(sock, ["get", "foo"])
        assert reply == "bar", f"Expected 'bar', got '{reply}'"
        print(color_text("PASS: get foo", "green"))
        ret, reply = send_command(sock, ["get", "baz"])
        assert reply == "baz not found", f"Expected 'baz not found', got '{reply}'"
        print(color_text("PASS: get baz", "green"))
        ret, reply = send_command(sock, ["del", "foo"])
        assert reply == "OK", f"Expected 'OK', got '{reply}'"
        print(color_text("PASS: del foo", "green"))
        ret, reply = send_command(sock, ["get", "foo"])
        assert reply == "foo not found", f"Expected 'foo not found', got '{reply}'"
        print(color_text("PASS: get foo (after delete)", "green"))
    print(color_text("String (key-value) tests passed!", "cyan"))

if __name__ == "__main__":
    test_string()
