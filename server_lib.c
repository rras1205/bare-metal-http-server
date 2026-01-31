#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h" // We link to our own header

// LOGIC 1: Setup the Server
SOCKET create_server_socket() {
    WSADATA wsaData;
    SOCKET server_socket;
    struct sockaddr_in server;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Winsock initialization failed.\n");
        return INVALID_SOCKET;
    }

    // Create Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        return INVALID_SOCKET;
    }

    // Prepare Address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        return INVALID_SOCKET;
    }
    
    // Listen
    listen(server_socket, 10);
    return server_socket;
}

// LOGIC 2: Handle the Client
DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    free(client_socket_ptr); // Free the pointer memory allocated in main

    char buffer[2000];
    recv(client_socket, buffer, 2000, 0);

    // Manual Routing Logic
    if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index ", 11) == 0) {
        FILE *f = fopen("index.html", "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);

            char *file_buffer = (char *)malloc(fsize + 1);
            fread(file_buffer, 1, fsize, f);
            fclose(f);

            char header[512];
            sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);
            
            send(client_socket, header, strlen(header), 0);
            send(client_socket, file_buffer, fsize, 0);
            free(file_buffer);
        } else {
            char response[] = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Missing index.html</h1>";
            send(client_socket, response, strlen(response), 0);
        }
    } 
    else if (strncmp(buffer, "GET /about ", 11) == 0 || strncmp(buffer, "GET /about/ ", 12) == 0) {
        char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>About Us</h1><p>This is a C server.</p>";
        send(client_socket, response, strlen(response), 0);
    }
    else {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Page Not Found</h1>";
        send(client_socket, response, strlen(response), 0);
    }

    // Print the thread ID so we know it's working concurrently
    printf("Request handled by Thread ID: %lu\n", GetCurrentThreadId());

    closesocket(client_socket);
    return 0;
}