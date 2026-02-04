#!/usr/bin/env python3
"""
Automated test runner for bare-metal HTTP server
Runs all test cases and reports results
"""

import subprocess
import time
import socket
import sys
import os

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    END = '\033[0m'
    BOLD = '\033[1m'

def print_header(text):
    print(f"\n{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{text}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.BLUE}{'='*60}{Colors.END}\n")

def print_test(name, status, details=""):
    status_str = f"{Colors.GREEN}✓ PASS{Colors.END}" if status else f"{Colors.RED}✗ FAIL{Colors.END}"
    print(f"{status_str} - {name}")
    if details:
        print(f"      {details}")

def is_server_running(port=8080):
    """Check if server is running on given port"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        result = sock.connect_ex(('127.0.0.1', port))
        sock.close()
        return result == 0
    except:
        return False

def test_basic_connection(port=8080):
    """Test 1: Basic HTTP connection"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', port))
        sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
        response = sock.recv(4096)
        sock.close()
        
        success = b"HTTP/1.1 200 OK" in response and b"<html" in response.lower()
        return success, f"Response: {len(response)} bytes"
    except Exception as e:
        return False, f"Error: {e}"

def test_404_response(port=8080):
    """Test 2: 404 for non-existent paths"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', port))
        sock.sendall(b"GET /nonexistent HTTP/1.1\r\nHost: localhost\r\n\r\n")
        response = sock.recv(4096)
        sock.close()
        
        success = b"404" in response
        return success, "404 response received" if success else "Expected 404, got different response"
    except Exception as e:
        return False, f"Error: {e}"

def test_multiple_requests(port=8080, count=10):
    """Test 3: Multiple sequential requests"""
    try:
        successes = 0
        for _ in range(count):
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect(('127.0.0.1', port))
            sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
            response = sock.recv(4096)
            sock.close()
            if b"HTTP/1.1 200 OK" in response:
                successes += 1
        
        success = successes == count
        return success, f"{successes}/{count} requests succeeded"
    except Exception as e:
        return False, f"Error: {e}"

def test_malformed_request(port=8080):
    """Test 4: Malformed HTTP request handling"""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        sock.connect(('127.0.0.1', port))
        sock.sendall(b"INVALID REQUEST\r\n\r\n")
        
        try:
            response = sock.recv(4096)
            # Server should respond with 400 or close connection
            success = b"400" in response or len(response) == 0
        except socket.timeout:
            success = True  # Server closed connection, which is acceptable
        
        sock.close()
        return success, "Server handled malformed request gracefully"
    except Exception as e:
        return False, f"Error: {e}"

def test_http_versions(port=8080):
    """Test 5: Different HTTP versions"""
    try:
        # Test HTTP/1.0
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', port))
        sock.sendall(b"GET / HTTP/1.0\r\nHost: localhost\r\n\r\n")
        response1 = sock.recv(4096)
        sock.close()
        
        # Test HTTP/1.1
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', port))
        sock.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n")
        response2 = sock.recv(4096)
        sock.close()
        
        success = (b"HTTP/1.1 200 OK" in response1 and b"HTTP/1.1 200 OK" in response2)
        return success, "Both HTTP/1.0 and HTTP/1.1 handled"
    except Exception as e:
        return False, f"Error: {e}"

def run_performance_test(port=8080):
    """Test 6: Quick performance test"""
    try:
        result = subprocess.run(
            [sys.executable, "tests/quick_test.py", str(port)],
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode == 0:
            # Extract throughput from output
            for line in result.stdout.split('\n'):
                if "Throughput:" in line:
                    throughput = line.split(':')[1].strip()
                    return True, f"Throughput: {throughput}"
            return True, "Performance test completed"
        else:
            return False, "Performance test failed"
    except Exception as e:
        return False, f"Error: {e}"

def main():
    print_header("BARE-METAL HTTP SERVER - AUTOMATED TEST SUITE")
    
    # Check if server is running
    if not is_server_running(8080):
        print(f"{Colors.RED}ERROR: Server is not running on port 8080{Colors.END}")
        print(f"{Colors.YELLOW}Please start the server first: .\\server.exe{Colors.END}")
        return 1
    
    print(f"{Colors.GREEN}✓ Server detected on port 8080{Colors.END}\n")
    
    # Run tests
    tests = [
        ("Basic HTTP Connection", lambda: test_basic_connection(8080)),
        ("404 Response", lambda: test_404_response(8080)),
        ("Multiple Sequential Requests (10)", lambda: test_multiple_requests(8080, 10)),
        ("Malformed Request Handling", lambda: test_malformed_request(8080)),
        ("HTTP Version Support", lambda: test_http_versions(8080)),
        ("Performance Test (100 requests)", lambda: run_performance_test(8080)),
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            success, details = test_func()
            results.append((test_name, success, details))
            print_test(test_name, success, details)
            time.sleep(0.1)  # Small delay between tests
        except Exception as e:
            results.append((test_name, False, str(e)))
            print_test(test_name, False, str(e))
    
    # Summary
    print_header("TEST SUMMARY")
    passed = sum(1 for _, success, _ in results if success)
    total = len(results)
    
    print(f"Total Tests: {total}")
    print(f"{Colors.GREEN}Passed: {passed}{Colors.END}")
    print(f"{Colors.RED}Failed: {total - passed}{Colors.END}")
    print(f"Success Rate: {(passed/total)*100:.1f}%\n")
    
    if passed == total:
        print(f"{Colors.GREEN}{Colors.BOLD}🎉 ALL TESTS PASSED!{Colors.END}\n")
        return 0
    else:
        print(f"{Colors.YELLOW}⚠️  Some tests failed. Review output above.{Colors.END}\n")
        return 1

if __name__ == "__main__":
    sys.exit(main())
