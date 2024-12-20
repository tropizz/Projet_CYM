#ifndef FONCTION_H
#define FONCTION_H

#include "common.h"

void listFiles(my_socket_t clientSocket);
void encryptFile(const char *filename, const char *key);
void encryptFiles(const char *directory);
int isRegularFile(const char *path);
void logError(const char *message);
void generateEncryptionKey(char *keyBuffer, size_t length);

#endif // FONCTION_H
