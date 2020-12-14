#include <stdio.h>
#include <string.h>

#include "utils.c"

#define SCORE 25 // Score à atteindre pour gagner. Maximum possible techniquement : 127.
#define MAX_PSEUDO_LENGTH 20
#define MAX_NB_JOUEURS 4
#define MAX_NB_PARTIES 25
#define MAX_NB_CONNEXIONS MAX_NB_JOUEURS * MAX_NB_PARTIES


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


struct tPartie {
    int id;
    int nbJoueurs;
    enum State state;
    int * joueursSockets;
    joueur * joueurs;
} typedef partie;


void ChaineDepuisJoueur(joueur * j, char output[])
{
    * output = (* j).id + 48;
    strncpy(output + 1, (* j).pseudo, MAX_PSEUDO_LENGTH);
}


void InitJoueurs(joueur joueurs[MAX_NB_JOUEURS])
{
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
        joueur j;
		j.id = i;
		strcpy(j.pseudo, "\0");
        j.avancee = 0;
        joueurs[i] = j;
	}
}


void InitPartie(partie * p)
{
    (* p).id = -1;
    InitJoueurs((* p).joueurs);
    for(int i = 0; i < MAX_NB_JOUEURS; i++)
    {
        (* p).joueursSockets[i] = -1;
    }
    (* p).nbJoueurs = 0;
    (* p).state = Init;
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


