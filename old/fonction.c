#include "common.h"
#include "fonction.h"

#define SIGNATURE "Ce fichier a bien été sécurisé aujourd'hui"

#ifdef _WIN32
#include <windows.h>

// Sur Windows, on utilise _stat
int isRegularFile(const char *path) {
    struct _stat pathStat;
    if (_stat(path, &pathStat) != 0) {
        fprintf(stderr, "Error retrieving file info\n");
        return 0;
    }
    return (pathStat.st_mode & S_IFREG) != 0;
}

void listFiles(my_socket_t clientSocket) {
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(".\\*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, strlen(error), 0);
        return;
    }
    do {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", findData.cFileName);
        send(clientSocket, buffer, strlen(buffer), 0);
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);

    const char *endMessage = "END_OF_LIST";
    send(clientSocket, endMessage, strlen(endMessage), 0);
}

#else
// Code Linux/Unix
int isRegularFile(const char *path) {
    struct stat pathStat;
    if (stat(path, &pathStat) != 0) {
        perror("Error retrieving file info");
        return 0;
    }
    return S_ISREG(pathStat.st_mode);
}

void listFiles(my_socket_t clientSocket) {
    DIR *d = opendir(".");
    if (!d) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, strlen(error), 0);
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name);
        send(clientSocket, buffer, strlen(buffer), 0);
    }
    closedir(d);

    const char *endMessage = "END_OF_LIST";
    send(clientSocket, endMessage, strlen(endMessage), 0);
}
#endif
