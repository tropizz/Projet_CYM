#include "common.h"
#include "fonction.h" // Inclusion pour la déclaration de listFiles

#define SERVER_IP "192.168.113.3" // Adresse IP du serveur
#define SERVER_PORT 8081

void printClientBanner() {
    printf("        ___            __\n");
    printf("  _____/ (_)__  ____  / /_\n");
    printf(" / ___/ / / _ \\/ __ \\/ __/\n");
    printf("/ /__/ / /  __/ / / / /_  \n");
    printf("\\___/_/_/\\___/_/ /_/\\__/  \n");
    printf("\n");
}

void handleCommand(my_socket_t clientSocket, const char *command) {
    if (strcmp(command, "run_list") == 0) {
        printf("Commande 'run_list' reçue. Exécution...\n");
        listFiles(clientSocket); // Envoi des fichiers listés au serveur
    } else {
        printf("Commande inconnue ou non prise en charge : %s\n", command);
    }
}

int main() {
    printClientBanner();

    // Initialisation de Winsock sous Windows
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Echec de l'initialisation de Winsock. Code d'erreur : %d\n", WSAGetLastError());
        return 1;
    }
#endif

    my_socket_t clientSocket;
    struct sockaddr_in serverAddr;

    // Création du socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("Echec de la création du socket");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
        WSACleanup();
#endif
        return 1;
    }

    // Configuration de l'adresse du serveur
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
        perror("Adresse IP invalide ou non supportée");
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Connexion au serveur
    printf("Tentative de connexion au serveur %s:%d...\n", SERVER_IP, SERVER_PORT);
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Echec de la connexion au serveur");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
#endif
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    printf("Connecté au serveur avec succès.\n");

    // Communication avec le serveur
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (recv(clientSocket, buffer, sizeof(buffer), 0) <= 0) {
            printf("Connexion fermée par le serveur.\n");
            break;
        }
        printf("Commande reçue : %s\n", buffer);
        if (strcmp(buffer, "exit") == 0) {
            printf("Commande 'exit' reçue. Fermeture...\n");
            break;
        }
        handleCommand(clientSocket, buffer);
    }

    // Fermeture du socket
    closesocket(clientSocket);
    printf("Socket fermé.\n");

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
