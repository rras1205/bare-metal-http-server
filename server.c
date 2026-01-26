#include <stdio.h>
#include <winsock2.h>
#include <string.h> // Needed for string comparison

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
    printf("Server is ready!\n");
    printf("Try visiting: http://localhost:8080/index\n");
    printf("Try visiting: http://localhost:8080/about\n");

    while(1) {
        printf("\nWaiting for incoming connections...\n");

        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) {
            printf("accept failed with error code : %d", WSAGetLastError());
            continue;
        }

        // 1. Read the Request
        recv(client_socket, buffer, 2000, 0);
        
        // 2. Manual Routing
        // We check the first few characters of the buffer.
        
        // If the request starts with "GET / " (The Home Page)
        // Note: we look for "GET / " (with a space)
        if (strncmp(buffer, "GET / ", 6) == 0 || strncmp(buffer, "GET /index ", 11) == 0) {
            
            char response[] = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html\r\n\r\n"
                              "<html><body><h1>HOME PAGE</h1>"
                              "<p>Welcome to the main area.</p></body></html>";
            send(client_socket, response, strlen(response), 0);
        
        } 
        // If the request starts with "GET /about "
        else if (strncmp(buffer, "GET /about ", 11) == 0) {
            
            char response[] = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/html\r\n\r\n"
                              "<html><body><h1>ABOUT US</h1>"
                              "<p>This server was built from scratch in C.</p></body></html>";
            send(client_socket, response, strlen(response), 0);
        
        } 
        // 404 Not Found for anything else
        else {
            
            char response[] = "HTTP/1.1 404 Not Found\r\n"
                              "Content-Type: text/html\r\n\r\n"
                              "<html><body><h1>404</h1>"
                              "<p>Page not found.</p></body></html>";
            send(client_socket, response, strlen(response), 0);
        }

        closesocket(client_socket);
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}pyty