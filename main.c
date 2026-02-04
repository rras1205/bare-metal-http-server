#include <stdio.h>
#include "server.h"

int main() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);

    server_socket = create_server_socket();
    if (server_socket == INVALID_SOCKET) return 1;

    // 1. Initialize the Pool (Use 8-16 threads usually)
    init_thread_pool(16);

    printf("High-Performance Server running on Port 8080...\n");

    while(1) {
        // 2. Accept
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) continue;

        // 3. Enqueue directly (No malloc, no CreateThread overhead)
        enqueue_client(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}