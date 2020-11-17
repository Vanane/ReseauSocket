#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void PrintAscii(char * c, int taille);
void SaisirString(int taille, char * output);


/**
 * Affiche à l'écran la valeur ASCII de chaque caractère d'une chaine.
 * @param c La chaine à afficher.
 */
void PrintAscii(char c[], int taille)
{
    for(int i = 0; i < taille; i++)
    {
        printf("%d ", c[i]);
    }
    printf("\n");
}


/**
 * Retourne une chaine de caractères de taille `taille`, avec formatage automatique.
 * @param taille Taille en caractères utiles, de la chaine à retourner.
 * @param output Tableau de caractères en sortie.
 */
void SaisirString(int taille, char * output)
{
    char saisie[taille+1];
    do
    {             
        fgets(saisie, taille + 1, stdin);
        if(strcmp(saisie, "\n") == 0)
            printf("Veuillez saisir une chaine de caractères !");
    }
    while((strcmp(saisie, "\n") == 0));

    * (saisie + strlen(saisie) - 1) = '\0';

    strcpy(output, saisie);
}


int min(int a, int b)
{
    return a < b ? a : b;
}


int max(int a, int b)
{
    return a > b ? a : b;
}
