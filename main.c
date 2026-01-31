#include <stdio.h>
#include <stdlib.h>
#include "server.h" // Import our tools

int main() {
    SOCKET server_socket, client_socket;
    struct sockaddr_in client;
    int c;

    // 1. Ask the library to create the server
    server_socket = create_server_socket();
    if (server_socket == INVALID_SOCKET) return 1;

    printf("Clean Code Server is running on Port 8080...\n");
    printf("Waiting for connections...\n");

    // 2. The Infinite Loop
    while(1) {
        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) continue;

        // Allocate memory for the socket pointer (needed for safe threading)
        SOCKET *new_sock = malloc(sizeof(SOCKET));
        *new_sock = client_socket;

        // 3. Delegate work to a new thread
        CreateThread(NULL, 0, handle_client, (LPVOID)new_sock, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}