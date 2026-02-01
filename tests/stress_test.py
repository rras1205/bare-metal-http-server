import threading
import time
import urllib.request

# CONFIGURATION
CONCURRENT_USERS = 50       # How many people hit the server at EXACTLY the same time
REQUESTS_PER_USER = 20      # How many times each person refreshes the page
TOTAL_REQUESTS = CONCURRENT_USERS * REQUESTS_PER_USER

def get_url(url):
    try:
        # Send a request and read the response (simulating a full download)
        with urllib.request.urlopen(url) as response:
            response.read()
    except Exception as e:
        print(f"Request failed: {e}")

def run_benchmark(port, name):
    url = f"http://localhost:{port}/"
    print(f"--- Testing {name} on Port {port} ---")
    print(f"Simulating {CONCURRENT_USERS} users making {REQUESTS_PER_USER} requests each...")
    
    threads = []
    start_time = time.time()

    # Spawn the "Users"
    for _ in range(CONCURRENT_USERS):
        # Create a thread that runs the loop
        t = threading.Thread(target=lambda: [get_url(url) for _ in range(REQUESTS_PER_USER)])
        threads.append(t)
        t.start()

    # Wait for everyone to finish
    for t in threads:
        t.join()

    end_time = time.time()
    duration = end_time - start_time
    rps = TOTAL_REQUESTS / duration

    print(f"Total Time: {duration:.2f} seconds")
    print(f"Throughput: {rps:.2f} Requests/Sec")
    print("-" * 30)

if __name__ == "__main__":
    # Test Python (Make sure it's running!)
    run_benchmark(9000, "Python Built-in")
    
    print("\n")
    
    # Test C (Make sure it's running!)
    run_benchmark(8080, "My C Server")