import socket
import time
import struct
import matplotlib.pyplot as plt

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 6969
NUM_REQUESTS = 1000

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

def benchmark():
    times = []
    successful_requests = 0

    for i in range(NUM_REQUESTS):
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((SERVER_HOST, SERVER_PORT))

            request = prepare_set_cmd(f"key{i}", f"value{i}")

            start_time = time.time()
            response = send_request(sock, request)
            end_time = time.time()

            elapsed_time = end_time - start_time
            times.append(elapsed_time)
            successful_requests += 1

            sock.close()
        except Exception as e:
            print(f"Request {i} failed: {e}")

    avg_time = sum(times) / successful_requests if successful_requests else float('inf')
    print(f"Total Requests: {NUM_REQUESTS}")
    print(f"Successful Requests: {successful_requests}")
    print(f"Total Time: {sum(times):.6f} sec")
    print(f"Average Time per Request: {avg_time:.6f} sec")

    # Plot the benchmark results
    plt.figure(figsize=(10, 5))
    plt.plot(range(len(times)), times, marker='o', linestyle='-', markersize=2, label="Request Time")
    plt.xlabel("Request Number")
    plt.ylabel("Time (seconds)")
    plt.title("Benchmark Fast")
    plt.legend()
    plt.grid()
    plt.savefig("benchmark_fast.png")  # Save the plot as an image

if __name__ == "__main__":
    benchmark()
