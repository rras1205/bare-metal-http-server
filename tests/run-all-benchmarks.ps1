# Comprehensive Benchmark Runner
# This script will guide you through benchmarking all three servers

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  C Server Efficiency Benchmark Suite  " -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "This script will help you benchmark:" -ForegroundColor Yellow
Write-Host "  1. Your C server (port 8080)" -ForegroundColor White
Write-Host "  2. Node.js server (port 3000)" -ForegroundColor White
Write-Host "  3. Python server (port 4000)" -ForegroundColor White
Write-Host ""

Write-Host "INSTRUCTIONS:" -ForegroundColor Green
Write-Host "1. Start each server in a SEPARATE terminal window" -ForegroundColor White
Write-Host "2. Run the benchmark command shown for each server" -ForegroundColor White
Write-Host "3. Record the results" -ForegroundColor White
Write-Host "4. Stop the server (Ctrl+C) before starting the next one" -ForegroundColor White
Write-Host ""

# Test 1: C Server
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "TEST 1: C Server (Multi-threaded)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Step 1: Open a NEW terminal and run:" -ForegroundColor Yellow
Write-Host "  .\server.exe" -ForegroundColor Green
Write-Host ""
Write-Host "Step 2: Once the server is running, press ENTER here to run the benchmark..." -ForegroundColor Yellow
$null = Read-Host

Write-Host "Running benchmark on C server..." -ForegroundColor Cyan
.\tests\benchmark.ps1 -port 8080 -name "C Server (16 threads)"
Write-Host ""
Write-Host "C Server benchmark complete! Record the Requests Per Second value." -ForegroundColor Green
Write-Host "Press ENTER to continue to Node.js test..." -ForegroundColor Yellow
$null = Read-Host

# Test 2: Node.js Server
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "TEST 2: Node.js Server (Single-threaded)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Step 1: STOP the C server (Ctrl+C in its terminal)" -ForegroundColor Yellow
Write-Host "Step 2: In the same terminal, run:" -ForegroundColor Yellow
Write-Host "  node tests\node-server.js" -ForegroundColor Green
Write-Host ""
Write-Host "Step 3: Once Node.js server is running, press ENTER here to benchmark..." -ForegroundColor Yellow
$null = Read-Host

Write-Host "Running benchmark on Node.js server..." -ForegroundColor Cyan
.\tests\benchmark.ps1 -port 3000 -name "Node.js Server"
Write-Host ""
Write-Host "Node.js benchmark complete! Record the Requests Per Second value." -ForegroundColor Green
Write-Host "Press ENTER to continue to Python test..." -ForegroundColor Yellow
$null = Read-Host

# Test 3: Python Server
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "TEST 3: Python Server (Single-threaded)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Step 1: STOP the Node.js server (Ctrl+C)" -ForegroundColor Yellow
Write-Host "Step 2: In the same terminal, run:" -ForegroundColor Yellow
Write-Host "  python tests\python-server.py" -ForegroundColor Green
Write-Host ""
Write-Host "Step 3: Once Python server is running, press ENTER here to benchmark..." -ForegroundColor Yellow
$null = Read-Host

Write-Host "Running benchmark on Python server..." -ForegroundColor Cyan
.\tests\benchmark.ps1 -port 4000 -name "Python Server"
Write-Host ""
Write-Host "Python benchmark complete! Record the Requests Per Second value." -ForegroundColor Green
Write-Host ""

# Summary
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  BENCHMARKING COMPLETE!" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Now compare your results to see C's efficiency advantage!" -ForegroundColor Green
Write-Host ""
Write-Host "Expected: C server should be 2-10x faster than Node.js and Python" -ForegroundColor Yellow
Write-Host "due to native compilation, multi-threading, and optimizations." -ForegroundColor Yellow
Write-Host ""
Write-Host "Don't forget to STOP the Python server (Ctrl+C) in its terminal." -ForegroundColor Yellow
Write-Host ""
