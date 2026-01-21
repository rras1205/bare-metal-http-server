#include <stdio.h>
#include <winsock2.h> // Windows Socket API

// This tells the compiler to use the "standard" input/output and the Windows Networking libraries.

int main() {
    WSADATA wsaData;
    SOCKET server_socket;

    // STEP 1: Initialize Winsock
    // Windows requires us to "turn on" the networking system before using it.
    // MAKEWORD(2,2) requests version 2.2 of Winsock.
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Winsock Initialized.\n");

    // STEP 2: Create a Socket
    // AF_INET      = IPv4 Address Family (not IPv6)
    // SOCK_STREAM  = TCP (Reliable, connection-based protocol)
    // 0            = Default Protocol (TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    } else {
        printf("Socket created successfully.\n");
    }

    // Cleanup (Good manners)
    closesocket(server_socket);
    WSACleanup();
    
    return 0;
}