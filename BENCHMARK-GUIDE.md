# Quick Benchmark Guide

## What's Been Set Up

✅ **C server recompiled** with HTTP parsing  
✅ **Node.js comparison server** created (`tests/node-server.js`)  
✅ **Python comparison server** created (`tests/python-server.py`)  
✅ **Automated benchmark runner** created (`tests/run-all-benchmarks.ps1`)

## Two Ways to Run Benchmarks

### Option 1: Automated Guide (Recommended)

Run the interactive benchmark script:

```powershell
.\tests\run-all-benchmarks.ps1
```

This will guide you through testing all three servers step-by-step.

### Option 2: Manual Testing

**Test 1: C Server**
```powershell
# Terminal 1: Start C server
.\server.exe

# Terminal 2: Benchmark it
.\tests\benchmark.ps1 -port 8080 -name "C Server (16 threads)"
```

**Test 2: Node.js Server**
```powershell
# Terminal 1: Stop C server (Ctrl+C), then start Node.js
node tests\node-server.js

# Terminal 2: Benchmark it
.\tests\benchmark.ps1 -port 3000 -name "Node.js Server"
```

**Test 3: Python Server**
```powershell
# Terminal 1: Stop Node.js (Ctrl+C), then start Python
python tests\python-server.py

# Terminal 2: Benchmark it
.\tests\benchmark.ps1 -port 4000 -name "Python Server"
```

## What to Look For

Record the **Requests Per Second** for each server:

- **C Server**: Should be highest (typically 1,000-5,000+ req/s)
- **Node.js**: Moderate (typically 200-800 req/s)
- **Python**: Lowest (typically 100-400 req/s)

## Why C is Faster

1. **Compiled native code** - No interpreter overhead
2. **Multi-threading** - 16 worker threads vs single-threaded
3. **In-memory caching** - Response pre-loaded
4. **Manual HTTP parsing** - No library overhead
5. **TCP_NODELAY** - Optimized socket settings
6. **Zero-copy operations** - Direct memory access

## Troubleshooting

**"node: command not found"**
- Install Node.js from https://nodejs.org
- Or skip Node.js test

**"python: command not found"**
- Try `py` instead of `python`
- Or install Python from https://python.org

**Port already in use**
- Make sure to stop previous server before starting next one

**Connection refused**
- Ensure server is running before benchmarking

## Next Steps

After recording results:
1. Calculate performance multipliers (C vs others)
2. Add results to README.md
3. Try stress test: `python tests\stress_test.py`
4. Experiment with different thread counts
