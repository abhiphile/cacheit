import socket
import time
from benchmark_utils import color_text

HOST = '127.0.0.1'
PORT = 6969
NUM_REQUESTS = 1000


def send_command(cmd):
    with socket.create_connection((HOST, PORT)) as sock:
        sock.sendall(cmd.encode())
        return sock.recv(4096).decode()

def benchmark_zset():
    zsetname = "benchzset"
    times = []
    for i in range(NUM_REQUESTS):
        cmd = f"zadd {zsetname} {i}.0 val{i}\n"
        start = time.time()
        send_command(cmd)
        end = time.time()
        times.append(end - start)
    avg = sum(times)/len(times)
    print(color_text(f"ZSet Benchmark: {NUM_REQUESTS} zadd ops, avg time: {avg:.6f}s", "blue"))

if __name__ == "__main__":
    benchmark_zset()
