#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void PrintAscii(char * c);
char * SaisirString();


/**
 * Affiche à l'écran la valeur ASCII de chaque caractère d'une chaine.
 * @param c La chaine à afficher.
 */
void PrintAscii(char * c)
{
    for(int i = 0; i < sizeof(c); i++)
    {
        printf("%d ", * (c + i));
    }
    printf("\n");
}


/**
 * Retourne une chaine de caractères de taille `taille`, avec formatage automatique.
 * @param taille Taille en caractères utiles, de la chaine à retourner.
 */
char * SaisirString(int taille)
{
    char * saisie = malloc(taille + 1);
    do
    {
        saisie = fgets(saisie, taille + 1, stdin);
        if(strcmp(saisie, "\n") == 0)
            printf("Veuillez saisir une chaine de caractères !");
    }
    while(strcmp(saisie, "\n") == 0);
    if(* (saisie + strlen(saisie) - 1) == '\n')
        * (saisie + strlen(saisie) - 1) = '\0';
    return saisie;
}
