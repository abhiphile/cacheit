from testlib import send_command, wait_for_server, color_text
import socket
from test_utils import print_separator
import uuid

def test_linkedlist():
    wait_for_server()
    print_separator("LinkedList tests")
    listname = f"mylist_{uuid.uuid4().hex[:8]}"
    with socket.create_connection(('127.0.0.1', 6969)) as sock:
        ret, reply = send_command(sock, ["lpush", listname, "42"])
        assert reply == "OK", f"Expected 'OK', got '{reply}'"
        print(color_text(f"PASS: lpush {listname} 42", "green"))
        ret, reply = send_command(sock, ["lpush", listname, "99"])
        assert reply == "OK", f"Expected 'OK', got '{reply}'"
        print(color_text(f"PASS: lpush {listname} 99", "green"))
        ret, reply = send_command(sock, ["lrange", listname, "0", "-1"])
        values = set(reply.split())
        print(f"DEBUG: lrange after lpush: {values}")
        assert "99" in values, f"Expected '99' in {values}"
        print(color_text(f"PASS: lrange {listname} 0 -1", "green"))
        ret, reply = send_command(sock, ["lrem", listname, "1", "42"])
        assert reply == "OK", f"Expected 'OK', got '{reply}'"
        print(color_text(f"PASS: lrem {listname} 1 42", "green"))
        ret, reply = send_command(sock, ["lrange", listname, "0", "-1"])
        values = set(reply.split())
        print(f"DEBUG: lrange after lrem: {values}")
        assert "99" in values and "42" not in values, f"Expected '99' in and '42' not in {values}"
        print(color_text(f"PASS: lrange {listname} 0 -1 (after remove)", "green"))
    print(color_text("LinkedList tests passed!", "cyan"))

if __name__ == "__main__":
    test_linkedlist()
