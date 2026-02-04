# Comprehensive Testing Guide

## Test Fairness Analysis

### Current Test Setup

**✅ What's Fair:**
- All servers serve the same `index.html` content
- All use in-memory caching (no disk I/O during requests)
- Same test scripts used for all servers
- Sequential test (`quick_test.py`) is fair - measures single-request performance

**⚠️ What's NOT Completely Fair:**

1. **Multi-threading vs Single-threading**
   - C server: 16 threads
   - Python/Node: Single-threaded
   - **Impact**: Not a pure language comparison, but realistic real-world scenario

2. **Connection-per-request overhead**
   - Test creates new TCP connection for each request
   - Measures connection setup overhead, not just request processing
   - Modern servers use HTTP keep-alive (connection reuse)

3. **Test methodology**
   - Sequential test: Doesn't utilize C's multi-threading advantage
   - Concurrent test: Python's multiprocessing has different overhead than C's threading

**Verdict**: Tests are fair for comparing **implementation approaches**, not pure language performance.

---

## Comprehensive Test Suite

### Test 1: Basic Functionality

**Purpose**: Verify server starts and responds to requests

**Steps:**
```bash
# 1. Compile
gcc -o server.exe main.c server_lib.c -lws2_32

# 2. Start server
.\server.exe

# Expected output:
# Loading index.html into memory cache...
# Cache Loaded. Size: XXX bytes.
# Thread Pool Initialized with 16 threads.
# High-Performance Server running on Port 8080...
```

**3. Test in browser:**
- Open `http://localhost:8080/` 
- Should see the green terminal-style page

**4. Test with curl:**
```bash
curl http://localhost:8080/
# Should return HTML content
```

**✅ Pass criteria:**
- Server starts without errors
- Browser shows index.html correctly
- curl returns HTTP 200 with HTML body

---

### Test 2: HTTP Method Handling

**Purpose**: Verify GET requests work, other methods handled

**Test Cases:**

```bash
# Test 2.1: GET request to root
curl -X GET http://localhost:8080/
# Expected: 200 OK with index.html

# Test 2.2: GET request to non-existent path
curl -X GET http://localhost:8080/nonexistent
# Expected: 404 Not Found

# Test 2.3: POST request (not implemented)
curl -X POST http://localhost:8080/
# Expected: Should still work (parsed as valid HTTP)
```

**✅ Pass criteria:**
- GET / returns 200 OK
- GET /other returns 404 Not Found
- Server doesn't crash on POST/PUT/DELETE

---

### Test 3: HTTP Parsing

**Purpose**: Verify manual HTTP parser handles various inputs

**Test Cases:**

```bash
# Test 3.1: Valid HTTP request
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc localhost 8080
# Expected: Valid response

# Test 3.2: HTTP/1.0 request
echo -e "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n" | nc localhost 8080
# Expected: Valid response (version parsed correctly)

# Test 3.3: Request with multiple headers
echo -e "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Test\r\nAccept: */*\r\n\r\n" | nc localhost 8080
# Expected: Valid response (headers parsed)
```

**✅ Pass criteria:**
- Parser handles HTTP/1.0 and HTTP/1.1
- Parser handles multiple headers
- No crashes on valid HTTP

---

### Test 4: Concurrent Connections

**Purpose**: Verify thread pool handles multiple simultaneous connections

**Test:**
```bash
python tests\quick_test.py 8080
```

**Expected output:**
```
Testing 127.0.0.1:8080 with 100 requests...
  Completed 10/100
  ...
  Completed 100/100

Total time: ~0.10s
Throughput: ~1000-1200 req/s
Avg latency: ~0.9-1.1 ms/req
```

**✅ Pass criteria:**
- All 100 requests succeed
- No connection refused errors
- Throughput > 800 req/s

---

### Test 5: Load Testing

**Purpose**: Verify server handles high load without crashing

**Test:**
```bash
python tests\stress_test.py 8080
```

**Expected output:**
```
--- CONCURRENT LOAD TEST: C Server (127.0.0.1:8080) ---
Spawning 8 processes sending 5000 requests each...
Total requests: 40000

--- RESULTS ---
Total Requests: 40000
Total Time:     ~90-100 seconds
Throughput:     ~400-450 Requests/Second
```

**✅ Pass criteria:**
- Server completes all 40,000 requests
- No crashes or hangs
- Throughput > 300 req/s

---

### Test 6: Error Handling

**Purpose**: Verify server handles malformed requests gracefully

**Test Cases:**

```bash
# Test 6.1: Malformed HTTP (missing version)
echo -e "GET /\r\n\r\n" | nc localhost 8080
# Expected: 400 Bad Request (or connection closed)

# Test 6.2: Invalid method
echo -e "INVALID / HTTP/1.1\r\n\r\n" | nc localhost 8080
# Expected: Server doesn't crash (may return 400 or process anyway)

# Test 6.3: Very long request line
python -c "print('GET /' + 'A'*10000 + ' HTTP/1.1\r\n\r\n')" | nc localhost 8080
# Expected: Server doesn't crash (buffer overflow protection)
```

**✅ Pass criteria:**
- Server doesn't crash on malformed input
- Returns 400 Bad Request or handles gracefully
- No buffer overflows

---

### Test 7: Memory Leak Check

**Purpose**: Verify no memory leaks over many requests

**Test:**
```bash
# Run stress test multiple times
python tests\stress_test.py 8080
# Wait for completion, then run again
python tests\stress_test.py 8080
# Repeat 3-5 times
```

**Monitor:**
- Task Manager → Server.exe memory usage
- Should stay relatively constant (not growing continuously)

**✅ Pass criteria:**
- Memory usage doesn't grow unbounded
- Server remains responsive after multiple stress tests

---

### Test 8: Comparison Testing

**Purpose**: Compare C server against Python server

**Test:**
```bash
# Test C server
python tests\quick_test.py 8080

# Stop C server (Ctrl+C)
# Start Python server
python tests\python-server.py

# Test Python server
python tests\quick_test.py 4000
```

**Expected results:**
- C server: ~1,100 req/s
- Python server: ~1,050 req/s
- Similar performance (within 10%)

**✅ Pass criteria:**
- Both servers handle 100 requests successfully
- Performance within same order of magnitude

---

### Test 9: Thread Pool Verification

**Purpose**: Verify thread pool is actually being used

**Test:**
```bash
# Start server
.\server.exe

# In another terminal, check thread count
# Windows: Task Manager → Details → server.exe → Right-click → Properties → Threads
# Should show ~17-18 threads (16 workers + main + system threads)
```

**✅ Pass criteria:**
- Thread count shows 16+ threads
- Threads persist across requests (not created per-request)

---

### Test 10: Port Binding

**Purpose**: Verify server handles port conflicts

**Test:**
```bash
# Start first server
.\server.exe

# Try to start second server (should fail)
.\server.exe
# Expected: Error - port already in use
```

**✅ Pass criteria:**
- Second server fails to start with clear error
- First server continues running

---

## Quick Test Checklist

Run these in order for comprehensive validation:

- [ ] **Compile**: `gcc -o server.exe main.c server_lib.c -lws2_32`
- [ ] **Start**: `.\server.exe` (verify startup messages)
- [ ] **Browser**: Open `http://localhost:8080/` (see HTML page)
- [ ] **404 Test**: Open `http://localhost:8080/test` (see 404)
- [ ] **Quick Test**: `python tests\quick_test.py 8080` (100 requests)
- [ ] **Stress Test**: `python tests\stress_test.py 8080` (40,000 requests)
- [ ] **Memory Check**: Run stress test 3x, monitor memory
- [ ] **Compare**: Test Python server with same scripts
- [ ] **Error Test**: Send malformed HTTP, verify no crash

---

## Test Fairness Recommendations

### For More Fair Comparisons:

**Option 1: Single-threaded C server**
```c
// In main.c, change:
init_thread_pool(1);  // Instead of 16
```
This makes it a fairer language comparison.

**Option 2: Multi-threaded Python**
Use `gunicorn` with multiple workers:
```bash
pip install gunicorn
gunicorn -w 16 -b localhost:4000 python-server:app
```

**Option 3: HTTP Keep-Alive Test**
Modify test to reuse connections instead of creating new ones.
This would show C's advantage better.

---

## Known Limitations

1. **Connection-per-request**: Test creates new TCP connection each time
   - Real-world servers use keep-alive
   - This adds ~0.5-1ms overhead per request

2. **Sequential testing**: Doesn't utilize multi-threading
   - C's 16 threads sit idle during sequential test
   - Need concurrent load to see threading benefit

3. **Small payload**: index.html is small (~650 bytes)
   - Doesn't test large file handling
   - Network/disk I/O not a factor

4. **No SSL/TLS**: Plain HTTP only
   - Real servers would have encryption overhead

5. **Windows-only**: Uses WinSock2
   - Not portable to Linux/Mac

---

## Success Criteria Summary

**Minimum Requirements:**
- ✅ Server starts without errors
- ✅ Serves index.html correctly
- ✅ Returns 404 for non-existent paths
- ✅ Handles 100 sequential requests
- ✅ Handles 40,000 concurrent requests
- ✅ No memory leaks
- ✅ No crashes on malformed input

**Bonus:**
- ✅ Performance within 10% of Python
- ✅ Thread pool verified (16 threads)
- ✅ Comparison tests documented

---

## Troubleshooting

**Server won't start:**
- Check if port 8080 is already in use
- Ensure index.html exists in project root
- Verify compilation succeeded

**Low performance:**
- Check if antivirus is scanning
- Ensure no debug logging enabled
- Verify thread pool size (should be 16)

**Connection errors:**
- Firewall may be blocking localhost
- Try running as administrator
- Check Windows Defender settings

**Memory leaks:**
- Run with debugger or memory profiler
- Check for missing `free()` calls
- Verify socket cleanup (closesocket)
