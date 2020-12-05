#include <stdio.h>
#include <string.h>

#include "utils.c"

#define SCORE 25 // Score à atteindre pour gagner. Maximum possible techniquement : 127.
#define MAX_PSEUDO_LENGTH 20
#define MAX_NB_JOUEURS 4


enum State
{
	Init,
    WaitForId,
	WaitForStart,
	Started,
	Paused,
	Finished
};


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


void ChaineDepuisJoueur(joueur * j, char output[])
{
    * output = (* j).id + 48;
    strncpy(output + 1, (* j).pseudo, MAX_PSEUDO_LENGTH);
}


void InitJoueurs(joueur joueurs[MAX_NB_JOUEURS])
{
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
		joueurs[i].id = i;
		strcpy(joueurs[i].pseudo, "\0");
        joueurs[i].avancee = 0;
	}
}


joueur JoueurDepuisChaine(char * chaine)
{
    joueur nouvJ;
    int index = 0;
    nouvJ.id = (* chaine + index) - 48; // -48 pour récupérer le chiffre du caractère ASCII
    nouvJ.avancee = 0;
    index++;
    strncpy(nouvJ.pseudo, chaine + index, min(MAX_PSEUDO_LENGTH, strlen(chaine + index)));
    return nouvJ;
}


void RecupererId(char * message, int * monId)
{
	* monId = (int) message[0] - 48;
}


