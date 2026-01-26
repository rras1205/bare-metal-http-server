#include <stdio.h>
#include <winsock2.h>

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[2000]; 

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Create Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    
    // Listen
    listen(server_socket, 3);
    printf("Server is running on http://localhost:8080 ...\n");

    // INFINITE LOOP STARTING 
    while(1) {
        printf("\nWaiting for incoming connections...\n");

        c = sizeof(struct sockaddr_in);
        client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
        
        if (client_socket == INVALID_SOCKET) {
            printf("accept failed with error code : %d", WSAGetLastError());
            continue; // Don't kill the server, just skip this connection
        }
        
        printf("Connection accepted!\n");

        // Read the request (we don't do anything with it yet, just clear the buffer)
        recv(client_socket, buffer, 2000, 0);

        // Send the Response
        char http_response[] = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "\r\n"
                               "<html><body>"
                               "<h1>My C Server is Running Forever!</h1>"
                               "<p>Refresh this page as many times as you want.</p>"
                               "</body></html>";

        send(client_socket, http_response, strlen(http_response), 0);
        printf("Response sent.\n");

        // Close the connection with THIS specific client, but keep the server open
        closesocket(client_socket);
    }
    // LOOP ENDS 

    closesocket(server_socket);
    WSACleanup();
    return 0;
}