// Code exfiltration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int aff_dossier(char dossier[]) {
    printf("Entre le repetoir a affficher : ");
    scanf("%s",dossier);
    strcat(dossier,"\\");
    struct dirent *dir;
    DIR *d = opendir(dossier);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
        return dossier;
    }
    else {
        printf("Erreur : Impossible d'ouvrir le dossier %s\n", dossier);
        return 1;
    }
}

int lect_fichier(char dossier[]) {
    char fichier[30], emplacement[130];
    printf("Choisir un fichier a lire : ");
    scanf("%s", fichier);
    snprintf(emplacement, sizeof(emplacement), "%s%s", dossier, fichier);
    printf("Fichier Choisi : %s \n", emplacement);
    FILE *f;
    char contenu;
    f=fopen(emplacement,"r+");
    if (f == NULL) {
        printf("Erreur: impossible d'ouvrir le fichier %s\n", fichier);
        return 1;
    }
    while((contenu=fgetc(f))!=EOF){
        //printf("%c",contenu);
        putchar(contenu);
    }
    fclose(f);
}

int fork() {

}

int main() {
    char dossier[100];
    aff_dossier(dossier);
    if (strlen(dossier) > 0) {
        lect_fichier(dossier);
    } else {
        printf("Erreur : dossier invalide.\n");
    }
    return 0;
}