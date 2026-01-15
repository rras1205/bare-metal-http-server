# bare-metal-http-server
Multi-threaded HTTP server that is built in C to analyze TCP/IP throughput

# why 
I wanted to understand how exactly the internet works in the packet level and this project is a reflection of that. 

# The Goal 
Build a functional HTTP server that is able to
1. Accept TCP connections.
2. Parse the raw HTTP strings manually. 
3. Serve static files
4. handle multiple users at one time through threading. 
5. then benchmark against a standard server in order to be able to prove the efficiency of C. 


## Current Status
- [x] Project Setup
- [ ] Socket Initialization 
- [ ] The Loop & Connections 
- [ ] Parsing HTTP 
- [ ] Benchmarking Test