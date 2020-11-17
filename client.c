/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include "conio/conio.c"
#include <pthread.h>

#include "course.c" // Divers types et fonctions en relation avec les entités du jeu de course

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


enum State
{
	Init,
	WaitForServer,
	WaitForStart,
	Started,
	Paused,
	Finished
};

void InfoJoueur(char * message, joueur joueurs[]);
void RecupererId(char * mesasge);
void DemarrerPartie(enum State s);
void PositionJoueurs(char * message, joueur joueurs[]);
void PauserJeu(enum State s);
void JoueurGagnant(char * message, enum State s);

void HandleMessage(char * message);
int main(int argc, char **argv);


int main(int argc, char **argv) {
	/* Variables liées au socket */
	int port = 12345;
	int 	socket_descriptor, 	/* descripteur de socket */
		longueur; 		/* longueur d'un buffer utilisé */
	sockaddr_in adresse_locale; 	/* adresse de socket local */
	hostent *	ptr_host; 		/* info sur une machine hote */
	servent *	ptr_service; 		/* info sur service */
	char 	buffer[256];
	char *	prog; 			/* nom du programme */
	char *	host; 			/* nom de la machine distante */


	/* Variables liées au jeu */
	enum State state = Init;

	joueur joueurs[4];
	char pseudo[MAX_PSEUDO_LENGTH];


	if (argc != 2) {
		perror("usage : client <adresse-serveur>");
		exit(1);
	}
   
	prog = argv[0];
	host = argv[1];
	
	printf("adresse du serveur  : %s \n", host);
	
	if ((ptr_host = gethostbyname(host)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son adresse.");
		exit(1);
	}
	
	/* copie caractere par caractere des infos de ptr_host vers adresse_locale */
	bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
	adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
	
	/* Utiliser un nouveau numero de port */
	adresse_locale.sin_port = htons(port);
	/*-----------------------------------------------------------*/
	
	printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
	
	/* creation de la socket */
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le serveur.");
		exit(1);
	}
	
	/* tentative de connexion au serveur dont les infos sont dans adresse_locale */
	if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de se connecter au serveur.");
		exit(1);
	}
	else
	{
		do
		{
			printf("Choisissez votre pseudo :\n");
			SaisirString(MAX_PSEUDO_LENGTH, pseudo);
			printf("Pseudo à envoyer :%s\n", pseudo);

			char message[1 + MAX_PSEUDO_LENGTH + 1];
			* message = '0';
			strcpy(message + 1, pseudo);

			printf("Message à envoyer : %s\n", message);

			int err;
			if((err = write(socket_descriptor, message, strlen(message) + 1)) != strlen(message) + 1)
				printf("Une erreur de communication est survenue : %d\n", err);
			else
			{
				state = WaitForStart;
			}
		} while (state != WaitForStart);
	}
	
	
	/**********************
	*  BOUCLE PRINCIPALE  *
	**********************/

	// Appelle la fonction adéquate pour prendre en charge le mesasge reçu, en fonction du code donné en message.
	// Côté Client : 
	//   0 = Le serveur envoie les informations d'un joueur
	//   1 = Le serveur répond au message Serveur 0, en envoyant l'ID de ce client
	//   2 = Le serveur envoie le signal "Démarrage de la partie"
	//   3 = Le serveur envoie la position des joueurs
	//   4 = Le serveur envoie le signal "Pause"
	//   5 = Le serveur envoie le signal "Joueur Gagnant"
	//   6 = Le serveur envoie les informations du joueur gagnant

	// Côté Serveur : 
	//   0 = Le client envoie ses informations (pseudo, IP)
	//   1 = Le client hôte envoie le signal "Démarrage de la partie"
	//   2 = Le client envoie le signal "Appui sur Espace"
	//   3 = Le client envoie le signal "Pause"
	//   4 = Le serveur envoie le signal "Joueur Gagnant"
	//   5 = Le serveur envoie les informations du joueur gagnant

	printf("Message envoyé, attente du signal pour commencer.\n");

	scanf("%s");
	char msg[2] = "1";
	write(socket_descriptor, msg, strlen(msg) + 1);
	sleep(3600);

	while(state == WaitForStart)
	{
		if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0)
		{
			switch(buffer[0])
			{
				case '0' : // Récupérer infos d'un joueur
					printf("%s\n", buffer);
					//InfoJoueur(buffer, joueurs);
				break;
				case '1' : // Récupérer ID du joueur
					RecupererId(buffer);
				break;
				case '2' : // Démarrer partie

				break;
				case '3' : // Position des joueurs

				break;
				case '4' : // Pause

				break;
				case '5' : // Vainqueur

				break;
			}
		}
	}


			  
	/* lecture de la reponse en provenance du serveur
	while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
	printf("reponse du serveur : \n");
	write(1,buffer,longueur);
	}
	
	printf("\nfin de la reception.\n");
	
	 */
	close(socket_descriptor);
	
	exit(0);
}


void InfoJoueur(char * message, joueur joueurs[])
{
	joueur nouvJ = JoueurDepuisChaine(message + 1);
	joueurs[nouvJ.id] = nouvJ;
}


void RecupererId(char * mesasge)
{

}


void DemarrerPartie(enum State s)
{

}


void PositionJoueurs(char * message, joueur joueurs[])
{

}


void PauserJeu(enum State s)
{

}


void JoueurGagnant(char * message, enum State s)
{

}

