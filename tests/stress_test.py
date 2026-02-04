import socket
import time
import multiprocessing
import sys

# CONFIGURATION
TARGET_IP = "127.0.0.1"
TARGET_PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
NUM_PROCESSES = 8       # Number of Python processes (Simulated Users)
REQUESTS_PER_PROCESS = 5000 # Total requests = 8 * 5000 = 40,000

def attack(pid):
    # Create a raw socket (Much faster than the 'requests' library)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((TARGET_IP, TARGET_PORT))
    except ConnectionRefusedError:
        print(f"Process {pid}: Connection Refused! Is server running?")
        return

    request = b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"
    start = time.time()
    
    for _ in range(REQUESTS_PER_PROCESS):
        sock.sendall(request)
        # We assume the server sends a response, we read a chunk to clear the buffer
        # This keeps the pipe flowing.
        sock.recv(4096)

    duration = time.time() - start
    sock.close()
    return (REQUESTS_PER_PROCESS, duration)

if __name__ == "__main__":
    server_name = "C Server" if TARGET_PORT == 8080 else f"Server on port {TARGET_PORT}"
    print(f"--- CONCURRENT LOAD TEST: {server_name} ({TARGET_IP}:{TARGET_PORT}) ---")
    print(f"Spawning {NUM_PROCESSES} processes sending {REQUESTS_PER_PROCESS} requests each...")
    print(f"Total requests: {NUM_PROCESSES * REQUESTS_PER_PROCESS}")
    
    start_time = time.time()
    
    # Use Multiprocessing to utilize all CPU cores
    with multiprocessing.Pool(processes=NUM_PROCESSES) as pool:
        results = pool.map(attack, range(NUM_PROCESSES))

    total_time = time.time() - start_time
    total_requests = NUM_PROCESSES * REQUESTS_PER_PROCESS
    
    print(f"\n--- RESULTS ---")
    print(f"Total Requests: {total_requests}")
    print(f"Total Time:     {total_time:.4f} seconds")
    print(f"Throughput:     {total_requests / total_time:.2f} Requests/Second")