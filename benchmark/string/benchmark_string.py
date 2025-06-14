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

def benchmark_string():
    times = []
    for i in range(NUM_REQUESTS):
        key = f"bench_key_{i}"
        value = f"bench_val_{i}"
        cmd = f"set {key} {value}\n"
        start = time.time()
        send_command(cmd)
        end = time.time()
        times.append(end - start)
    avg = sum(times)/len(times)
    print(color_text(f"String Benchmark: {NUM_REQUESTS} set ops, avg time: {avg:.6f}s", "cyan"))

if __name__ == "__main__":
    benchmark_string()
