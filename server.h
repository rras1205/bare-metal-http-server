#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

// HTTP Request Structure
typedef struct {
    char method[16];        // GET, POST, etc.
    char path[256];         // /index.html, /api/data, etc.
    char version[16];       // HTTP/1.1
    char host[128];         // Host header value
    int content_length;     // Content-Length header value
} HttpRequest;

// Parse raw HTTP request string
int parse_http_request(const char *buffer, HttpRequest *req);

// Initialize the server socket
SOCKET create_server_socket();

// Start the worker threads
void init_thread_pool(int thread_count);

// Add a client to the work queue (Main thread calls this)
void enqueue_client(SOCKET client_socket);

#endif