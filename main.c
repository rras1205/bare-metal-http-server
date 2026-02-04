#include <stdio.h>
#include "server.h"

int main() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);

    server_socket = create_server_socket();
    if (server_socket == INVALID_SOCKET) return 1;

    // Pre-allocate thread pool to avoid CreateThread() overhead per request.
    // 16 threads chosen to match typical CPU core count for optimal parallelism.
    // Thread-per-request would be expensive: 40,000 requests = 40,000 thread creations!
    init_thread_pool(16);

    printf("High-Performance Server running on Port 8080...\n");

    // Main thread's only job: accept connections and enqueue them.
    // This keeps accept() fast and non-blocking for new connections.
    while(1) {
        // accept() blocks until a client connects, then returns a new socket.
        // This socket represents the TCP connection to that specific client.
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) continue;

        // Hand off to worker thread pool. No malloc, no thread creation.
        // Worker threads wake up from condition variable, process, and sleep again.
        enqueue_client(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}