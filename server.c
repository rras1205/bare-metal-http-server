#include <stdio.h>
#include <winsock2.h>
#include <string.h>

// Threading
// We need  specific function signature for Windows threads:
// must return a DWORD and accept an LPVOID (void pointer).
DWORD WINAPI handle_client(LPVOID client_socket_ptr) {
    // Retrieve the socket from the pointer
    SOCKET client_socket = *(SOCKET*)client_socket_ptr;
    free(client_socket_ptr); // Free the memory we allocated in main()

    char buffer[2000];
    recv(client_socket, buffer, 2000, 0);

    // 2. Logic (Same as before, just inside a thread now)
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
            sprintf(header, "HTTP/1.1 200s OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);
            
            send(client_socket, header, strlen(header), 0);
            send(client_socket, file_buffer, fsize, 0);
            free(file_buffer);
        } else {
            char response[] = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 Missing index.html</h1>";
            send(client_socket, response, strlen(response), 0);
        }
    } 
    else {
        char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Page Not Found</h1>";
        send(client_socket, response, strlen(response), 0);
    }

    printf("Thread %lu handled the request.\n", GetCurrentThreadId()); // Optional: See it working

    closesocket(client_socket);
    return 0;
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) return 1;
    
    listen(server_socket, 10); // Queue up to 10 connections
    printf("Multithreaded Server running on Port 8080...\n");

    while(1) {
        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) continue;

        // Manual Memory Allocation for Threads
        // We cannot just pass & client_socket because the loop might overwrite it 
        // before the thread reads it. We must allocate new memory for every connection.
        SOCKET *new_sock = malloc(sizeof(SOCKET));
        *new_sock = client_socket;

        // Create the Thread
        // 1. Security (NULL)
        // 2. Stack Size (0 = default)
        // 3. Function to run (handle_client)
        // 4. Argument to pass (new_sock)
        // 5. Flags (0 = run immediately)
        // 6. Thread ID (NULL = ignore)
        CreateThread(NULL, 0, handle_client, (LPVOID)new_sock, 0, NULL);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}