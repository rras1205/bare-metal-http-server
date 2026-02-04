#include "server.h"
#include <process.h> // For _beginthreadex if needed, but CreateThread is fine here
#include <string.h>

// --- HTTP PARSER ---
// Manual HTTP parsing without libraries to understand the protocol.
// HTTP format: "METHOD /path HTTP/version\r\nHeader: value\r\n\r\n"
// We use pointer arithmetic and strchr() for efficiency.
int parse_http_request(const char *buffer, HttpRequest *req) {
    // Zero out struct to ensure no garbage values
    memset(req, 0, sizeof(HttpRequest));
    req->content_length = -1;  // -1 means "not specified"
    
    // Parse Request Line: "METHOD /path HTTP/version\r\n"
    const char *line_start = buffer;
    const char *space1 = strchr(line_start, ' ');
    if (!space1) return -1;
    
    // Extract METHOD
    int method_len = space1 - line_start;
    if (method_len >= sizeof(req->method)) method_len = sizeof(req->method) - 1;
    strncpy(req->method, line_start, method_len);
    req->method[method_len] = '\0';
    
    // Extract PATH
    const char *space2 = strchr(space1 + 1, ' ');
    if (!space2) return -1;
    int path_len = space2 - (space1 + 1);
    if (path_len >= sizeof(req->path)) path_len = sizeof(req->path) - 1;
    strncpy(req->path, space1 + 1, path_len);
    req->path[path_len] = '\0';
    
    // Extract VERSION
    const char *line_end = strstr(space2, "\r\n");
    if (!line_end) return -1;
    int version_len = line_end - (space2 + 1);
    if (version_len >= sizeof(req->version)) version_len = sizeof(req->version) - 1;
    strncpy(req->version, space2 + 1, version_len);
    req->version[version_len] = '\0';
    
    // Parse Headers
    const char *header_start = line_end + 2;
    while (1) {
        // Check for end of headers (empty line)
        if (header_start[0] == '\r' && header_start[1] == '\n') break;
        
        const char *colon = strchr(header_start, ':');
        if (!colon) break;
        
        const char *header_end = strstr(header_start, "\r\n");
        if (!header_end) break;
        
        // Calculate header name length
        int name_len = colon - header_start;
        
        // Skip whitespace after colon
        const char *value_start = colon + 1;
        while (*value_start == ' ') value_start++;
        
        int value_len = header_end - value_start;
        
        // Parse specific headers we care about
        if (name_len == 4 && strncmp(header_start, "Host", 4) == 0) {
            if (value_len >= sizeof(req->host)) value_len = sizeof(req->host) - 1;
            strncpy(req->host, value_start, value_len);
            req->host[value_len] = '\0';
        }
        else if (name_len == 14 && strncmp(header_start, "Content-Length", 14) == 0) {
            char len_buf[32];
            if (value_len >= sizeof(len_buf)) value_len = sizeof(len_buf) - 1;
            strncpy(len_buf, value_start, value_len);
            len_buf[value_len] = '\0';
            req->content_length = atoi(len_buf);
        }
        
        header_start = header_end + 2;
    }
    
    return 0;
}

// --- CACHE VARS ---
// Pre-load index.html into memory at startup.
// Why? Reading from disk per request would be 100x slower.
// We also pre-build the HTTP headers to avoid sprintf() in hot path.
char *CACHED_RESPONSE = NULL;
int CACHED_SIZE = 0;

// --- THREAD POOL VARS ---
// Circular queue to hold pending client sockets.
// Why 5000? Large enough to handle burst traffic without dropping connections.
// Bounded to prevent memory exhaustion under attack/overload.
#define QUEUE_SIZE 5000     // Can hold 5000 pending connections
SOCKET client_queue[QUEUE_SIZE];
int q_front = 0, q_rear = 0, q_count = 0;  // Circular buffer indices

// Synchronization primitives for thread-safe queue access.
// CRITICAL_SECTION: Mutex for protecting queue state.
// CONDITION_VARIABLE: Allows threads to sleep efficiently when queue is empty.
CRITICAL_SECTION queue_lock;
CONDITION_VARIABLE queue_cond;

// Pre-load index.html into memory at startup.
// Why? Reading from disk per request would be 100x slower.
// We also pre-build the HTTP headers to avoid sprintf() in hot path.
void load_cache() {
    printf("Loading index.html into memory cache...\n");
    FILE *f = fopen("index.html", "rb");
    if (!f) {
        // Fallback if file missing
        char *fallback = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World";
        CACHED_SIZE = strlen(fallback);
        CACHED_RESPONSE = (char *)malloc(CACHED_SIZE + 1);
        strcpy(CACHED_RESPONSE, fallback);
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *file_buffer = (char *)malloc(fsize);
    fread(file_buffer, 1, fsize, f);
    fclose(f);
    
    char header[512];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);
    CACHED_SIZE = strlen(header) + fsize;
    CACHED_RESPONSE = (char *)malloc(CACHED_SIZE);
    memcpy(CACHED_RESPONSE, header, strlen(header));
    memcpy(CACHED_RESPONSE + strlen(header), file_buffer, fsize);
    free(file_buffer);
    printf("Cache Loaded. Size: %d bytes.\n", CACHED_SIZE);
}

// Worker thread function - each thread runs this loop forever.
// Pattern: Wait → Grab work → Process → Repeat
// This is more efficient than creating/destroying threads per request.
DWORD WINAPI thread_worker(LPVOID lpParam) {
    char buffer[1024];  // Stack-allocated buffer, no malloc needed

    while (1) {
        SOCKET client_socket;

        // === PHASE 1: Acquire work from queue ===
        EnterCriticalSection(&queue_lock);

        // If queue is empty, sleep on condition variable.
        // This is efficient: thread doesn't burn CPU while waiting.
        // Main thread will wake us with WakeConditionVariable().
        while (q_count == 0) {
            SleepConditionVariableCS(&queue_cond, &queue_lock, INFINITE);
        }

        // Dequeue from circular buffer
        client_socket = client_queue[q_front];
        q_front = (q_front + 1) % QUEUE_SIZE;  // Wrap around using modulo
        q_count--;

        LeaveCriticalSection(&queue_lock);

        // === PHASE 2: Process request (outside lock for concurrency) ===
        int bytes = recv(client_socket, buffer, 1024, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            
            HttpRequest req;
            if (parse_http_request(buffer, &req) == 0) {
                // Route based on method and path.
                // Production servers would have routing tables, regex matching, etc.
                if (strcmp(req.method, "GET") == 0 && strcmp(req.path, "/") == 0) {
                    // Send pre-built response (headers + body in one buffer)
                    send(client_socket, CACHED_RESPONSE, CACHED_SIZE, 0);
                } else {
                    // 404 for other paths
                    char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
                    send(client_socket, not_found, strlen(not_found), 0);
                }
            } else {
                // Failed to parse, send 400 Bad Request
                char *bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Length: 11\r\n\r\nBad Request";
                send(client_socket, bad_request, strlen(bad_request), 0);
            }
        }
        closesocket(client_socket);
    }
    return 0;
}

// Initialize thread pool at startup.
// All threads are created once and reused for the lifetime of the server.
void init_thread_pool(int thread_count) {
    InitializeCriticalSection(&queue_lock);
    InitializeConditionVariable(&queue_cond);

    // Create worker threads. They immediately start running thread_worker().
    // Each thread will sleep on the condition variable until work arrives.
    for (int i = 0; i < thread_count; i++) {
        CreateThread(NULL, 0, thread_worker, NULL, 0, NULL);
    }
    printf("Thread Pool Initialized with %d threads.\n", thread_count);
}

// Called by main thread to add new client to work queue.
// This is the producer in a producer-consumer pattern.
void enqueue_client(SOCKET client_socket) {
    EnterCriticalSection(&queue_lock);

    // Trade-off: Drop connections vs block accept().
    // We choose to drop because blocking would prevent new connections.
    // In practice, 5000 queue slots is plenty for burst traffic.
    if (q_count < QUEUE_SIZE) {
        client_queue[q_rear] = client_socket;
        q_rear = (q_rear + 1) % QUEUE_SIZE;  // Circular buffer wrap
        q_count++;
        // Wake exactly ONE sleeping worker (not all - that would be wasteful)
        WakeConditionVariable(&queue_cond);
    } else {
        // Queue full - server is overloaded. Drop this connection.
        // Better than crashing or running out of memory.
        closesocket(client_socket);
    }

    LeaveCriticalSection(&queue_lock);
}

SOCKET create_server_socket() {
    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    load_cache(); 

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // TCP_NODELAY disables Nagle's algorithm.
    // Nagle buffers small packets to reduce network overhead.
    // For HTTP, we want immediate sends even if small (lower latency).
    // Trade-off: Lower latency vs slightly more network packets.
    int flag = 1;
    setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    bind(server_socket, (struct sockaddr *)&server, sizeof(server));
    listen(server_socket, SOMAXCONN);
    
    return server_socket;
}