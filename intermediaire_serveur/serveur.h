#ifndef SERVEUR_H
#define SERVEUR_H

#include "common.h"

// Définition des constantes
#define SERVER_PORT 8081

// Déclaration des fonctions
void printServerBanner();
void handleClient(my_socket_t clientSocket);

#endif // SERVEUR_H
