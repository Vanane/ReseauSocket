#include <stdio.h>
#include <string.h>
#include "utils.c"
#define SCORE 50 // Score à atteindre pour gagner
#define MAX_PSEUDO_LENGTH 20




// Struct représentant un joueur de la course
struct tJoueur
{
    int id;
    char pseudo[MAX_PSEUDO_LENGTH];
    int avancee;
} typedef joueur;


// Struct correspondant à une fonction pour prendre en charge un message reçu
struct tMessage
{
    void (*handler)(char *);
} typedef message;


char * ChaineDepuisJoueur(joueur j)
{
    char * ret = malloc(1 + MAX_PSEUDO_LENGTH + 1);
    * ret = j.id;
    strncpy(ret + 1, j.pseudo, MAX_PSEUDO_LENGTH);
}

joueur JoueurDepuisChaine(char * chaine)
{
    joueur nouvJ;
    int index = 0;
    nouvJ.id = (* chaine + index);
    nouvJ.avancee = 0;
    index++;
    strncpy(nouvJ.pseudo, chaine + index, min(MAX_PSEUDO_LENGTH, strlen(chaine + index)));
    return nouvJ;
}
