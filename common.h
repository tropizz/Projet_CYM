#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Lier la bibliothèque Winsock pour Windows

    // Définition des types et constantes spécifiques à Windows
    typedef SOCKET my_socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    #define closesocket closesocket

    // Pour la gestion des fichiers et répertoires sous Windows
    #include <sys/types.h>
    #include <sys/stat.h> // Pour _stat et _S_IFREG

#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    // Inclusions spécifiques à Linux/Unix
    #include <dirent.h>  // Pour opendir, readdir, closedir
    #include <sys/stat.h> // Pour stat et S_ISREG

    // Définition des types et constantes spécifiques à Linux/Unix
    typedef int my_socket_t;
    #define INVALID_SOCKET_VALUE (-1)
    #define SOCKET_ERROR_VALUE   (-1)
    #define closesocket close

#endif // _WIN32

#endif // COMMON_H
