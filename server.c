#include <stdio.h>
#include <winsock2.h>

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

    printf("Waiting for incoming connections...\n");
    
    c = sizeof(struct sockaddr_in);
    client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
    
    recv(client_socket, buffer, 2000, 0);
    printf("%s\n", buffer); // Print request to terminal

    
    // 1. Define the Response
    // We must manually format the HTTP headers. 
    // \r\n is the standard "Enter" key for the internet.
    char http_response[] = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n" // We are sending HTML now!
                           "\r\n" // The empty line that separates headers from body
                           "<html><body>"
                           "<h1>Hello from C!</h1>"
                           "<p>This page is being served by a raw socket server.</p>"
                           "</body></html>";

    // 2. Send the Response
    send(client_socket, http_response, strlen(http_response), 0);
    printf("Response sent to browser.\n");


    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}