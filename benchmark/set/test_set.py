import socket
import pytest

def send_command(cmd):
    with socket.create_connection(("127.0.0.1", 6969)) as sock:
        sock.sendall(cmd.encode())
        return sock.recv(4096).decode()

def test_set_ops():
    pass
