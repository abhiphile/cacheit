import socket
import time
import struct
import matplotlib.pyplot as plt
import statistics
import concurrent.futures

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 6969
NUM_REQUESTS = 5000  # Increased for extensive benchmarking
THREADS = 10  # Number of concurrent threads


def prepare_set_cmd(key, value):
    nargs = 3
    SET = b"set"
    key = key.encode()
    value = value.encode()

    message = struct.pack("!I", nargs)
    message += struct.pack("!I", len(SET)) + SET
    message += struct.pack("!I", len(key)) + key
    message += struct.pack("!I", len(value)) + value

    return message


def send_request(sock, request):
    sock.sendall(request)
    response = sock.recv(4096)
    return response


def benchmark_request(i):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_HOST, SERVER_PORT))
        request = prepare_set_cmd(f"key{i}", f"value{i}")

        start_time = time.time()
        response = send_request(sock, request)
        end_time = time.time()

        sock.close()
        return end_time - start_time
    except Exception as e:
        print(f"Request {i} failed: {e}")
        return None


def benchmark():
    times = []

    with concurrent.futures.ThreadPoolExecutor(max_workers=THREADS) as executor:
        results = list(executor.map(benchmark_request, range(NUM_REQUESTS)))

    times = [t for t in results if t is not None]

    if times:
        avg_time = sum(times) / len(times)
        median_time = statistics.median(times)
        max_time = max(times)
        min_time = min(times)
    else:
        avg_time = median_time = max_time = min_time = float('inf')

    print(f"Total Requests: {NUM_REQUESTS}")
    print(f"Successful Requests: {len(times)}")
    print(f"Total Time: {sum(times):.6f} sec")
    print(f"Average Time per Request: {avg_time:.6f} sec")
    print(f"Median Time per Request: {median_time:.6f} sec")
    print(f"Max Time per Request: {max_time:.6f} sec")
    print(f"Min Time per Request: {min_time:.6f} sec")

    # Plot the benchmark results
    plt.figure(figsize=(10, 5))
    plt.plot(range(len(times)), times, marker='o', linestyle='-', markersize=2, label="Request Time")
    plt.xlabel("Request Number")
    plt.ylabel("Time (seconds)")
    plt.title("Benchmark Slow")
    plt.legend()
    plt.grid()
    plt.savefig("benchmark_slow.png")  # Save the plot as an image


if __name__ == "__main__":
    benchmark()
