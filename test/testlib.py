import socket
import struct
import time

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
    retcode = struct.unpack('<I', sock.recv(4))[0]
    length = struct.unpack('<I', sock.recv(4))[0]
    data = b''
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            break
        data += chunk
    return retcode, data.decode()

def wait_for_server():
    for _ in range(10):
        try:
            with socket.create_connection((HOST, PORT)):
                return
        except Exception:
            time.sleep(0.5)
    raise RuntimeError("Server not available")

def color_text(text, color):
    colors = {
        'red': '\033[91m',
        'green': '\033[92m',
        'yellow': '\033[93m',
        'blue': '\033[94m',
        'magenta': '\033[95m',
        'cyan': '\033[96m',
        'reset': '\033[0m',
    }
    return f"{colors.get(color, '')}{text}{colors['reset']}"
