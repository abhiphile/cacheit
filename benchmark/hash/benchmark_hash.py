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

def benchmark_hash():
    hashname = "benchhash"
    times = []
    for i in range(NUM_REQUESTS):
        cmd = f"hset {hashname} field{i} val{i}\n"
        start = time.time()
        send_command(cmd)
        end = time.time()
        times.append(end - start)
    avg = sum(times)/len(times)
    print(color_text(f"Hash Benchmark: {NUM_REQUESTS} hset ops, avg time: {avg:.6f}s", "yellow"))

if __name__ == "__main__":
    benchmark_hash()
