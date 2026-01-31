#include <stdio.h>
#include <winsock2.h>
#include <string.h>

// Manual File Loader
// We can't just "send" a file - we have to read it into memory first.
void send_file(SOCKET client_socket, const char* filename) {
    FILE *f = fopen(filename, "rb"); // "rb" = Read Binary (important for images/files)
    
    if (f == NULL) {
        // If file not found, send 404
        char not_found[] = "HTTP/1.1 404 Not Found\r\n\r\n<h1>404 File Not Found</h1>";
        send(client_socket, not_found, strlen(not_found), 0);
        return;
    }

    // Calculate File Size Manually
    // We jump to the end (SEEK_END), check the position (ftell), then jump back to start (SEEK_SET).
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Manual Memory Management
    // We assume a max file size of 1MB for this demo (just to keep it simple).
    char *file_buffer = (char *)malloc(fsize + 1);
    fread(file_buffer, 1, fsize, f);
    fclose(f);

    // Prepare the HTTP Header
    // We must state the Content-Length so the browser knows when the file ends.
    char header[512];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", fsize);

    // Send Header first
    send(client_socket, header, strlen(header), 0);

    // Send Body (The File Data)
    send(client_socket, file_buffer, fsize, 0);

    // Clean up memory
    free(file_buffer);
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[2000]; 

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    
    listen(server_socket, 3);
    printf("Server is ready at http://localhost:8080\n");

    while(1) {
        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) {
            continue;
        }

        // Read Request
        recv(client_socket, buffer, 2000, 0);
        
        // Routing Logic
        if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index ", 11) == 0) {
            // Serve the file
            send_file(client_socket, "index.html");
        } 
        else if (strncmp(buffer, "GET /about ", 11) == 0) {
            // Keep the hardcoded About page just to show the difference
            char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>About Us</h1><p>Hardcoded response.</p>";
            send(client_socket, response, strlen(response), 0);
        } 
        else {
            char response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404</h1>";
            send(client_socket, response, strlen(response), 0);
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}