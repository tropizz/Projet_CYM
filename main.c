// Code exfiltration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int main() {
    struct dirent *dir;
    DIR *d = opendir("C:\\Users\\tomiv\\OneDrive\\PC-Perso\\Documents");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    char fichier[30];
    printf("Choisir un fichier a lire : ");
    //getchar();
    scanf("%s", fichier);
    printf(" Fichier Choisi : %s \n", fichier);
    FILE *f;
    char contenu;

    f=fopen(fichier,"r");
    if (f == NULL) {
      printf("Erreur: impossible d'ouvrir le fichier %s\n", fichier);
      return 1;
    }
    while((contenu=fgetc(f))!=EOF){
        //printf("%c",contenue);
        putchar(contenu);
    }
    fclose(f);
    return 0;
}