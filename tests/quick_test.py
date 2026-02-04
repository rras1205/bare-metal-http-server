import socket
import time
import sys

TARGET_IP = "127.0.0.1"
TARGET_PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
NUM_REQUESTS = 100

request = b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"

print(f"Testing {TARGET_IP}:{TARGET_PORT} with {NUM_REQUESTS} requests...")
start = time.time()

for i in range(NUM_REQUESTS):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((TARGET_IP, TARGET_PORT))
        sock.sendall(request)
        response = sock.recv(4096)
        sock.close()
        if (i + 1) % 10 == 0:
            print(f"  Completed {i + 1}/{NUM_REQUESTS}")
    except Exception as e:
        print(f"  Error on request {i + 1}: {e}")

duration = time.time() - start
print(f"\nTotal time: {duration:.2f}s")
print(f"Throughput: {NUM_REQUESTS / duration:.2f} req/s")
print(f"Avg latency: {(duration / NUM_REQUESTS * 1000):.2f} ms/req")
