#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <windows.h> // Needed for thread functions

// FUNCTION PROTOTYPES
// This acts as a "contract". Any file that includes this header
// knows these functions exist, even if it doesn't know how they work.

// Function to initialize Winsock, create socket, bind, and listen
SOCKET create_server_socket();

// Function to handle individual client interactions (threaded)
DWORD WINAPI handle_client(LPVOID client_socket_ptr);

#endif