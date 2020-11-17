/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include <pthread.h>

#include "course.c" // Divers types et fonctions en relation avec les entités du jeu de course

#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;



enum ServerState
{
	Init,
	WaitPlayers,
	Starting,
	Playing,
	Paused,
	Finished
};


struct TthreadArgs {
	enum ServerState * state;
	joueur * j;
	int fd;
} typedef threadArgs;


void * ThreadJoueur(threadArgs * args);
int main(int argc, char **argv);
void test(int * t);


int main(int argc, char **argv) {
	/* Variables liées au socket */
	int port = 12345;
	int socket_descriptor, 		/* descripteur de socket */
		nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
		longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
	sockaddr_in adresse_locale, 		/* structure d'adresse locale*/
		adresse_client_courant; 	/* adresse client courant */
	hostent* ptr_hote; 			/* les infos recuperees sur la machine hote */
	servent* ptr_service; 			/* les infos recuperees sur le service de la machine */
	char machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
	char buffer[256];
	int longueur;

	pthread_t threads[4];

	/* Variables liées au jeu */
	joueur joueurs[4]; /* Tableau contenant les joueurs */
	int nbJoueurs = 0;

	enum ServerState state = Init;

	
	gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
	
	/* recuperation de la structure d'adresse en utilisant le nom */
	if ((ptr_hote = gethostbyname(machine)) == NULL) {
	  perror("erreur : impossible de trouver le serveur a partir de son nom.");
	  exit(1);
	}
	
	/* initialisation de la structure adresse_locale avec les infos recuperees */			
	
	/* copie de ptr_hote vers adresse_locale */
	bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
	adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
	adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

	/*-----------------------------------------------------------*/
	/* Utiliser un nouveau numero de port */
	adresse_locale.sin_port = htons(port);
	/*-----------------------------------------------------------*/
	
	printf("numero de port pour la connexion au serveur : %d \n", 
		ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
	
	/* creation de la socket */
	if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
	}

	/* association du socket socket_descriptor à la structure d'adresse adresse_locale */
	if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
	}
	
	/* initialisation de la file d'ecoute */
	listen(socket_descriptor,5);


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
	
	
	/*pthread_t thread;
	
	pthread_create(&thread, NULL, &test, &nouv_socket_descriptor);

	*/
	state = WaitPlayers;
	while(state == WaitPlayers)
	{
		longueur_adresse_courante = sizeof(adresse_client_courant);
		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if((nouv_socket_descriptor = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0)
		{
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}
		else
		{
			threadArgs args =
			{
				&state, &(joueurs[nbJoueurs]), nouv_socket_descriptor
			};
			pthread_create(&(threads[nbJoueurs]), NULL, &ThreadJoueur, &args);
		}
		
		if((longueur = read(nouv_socket_descriptor, buffer, sizeof(buffer))) > 0) 
		{
			printf("message reçu : %s\n", buffer);
			switch(buffer[0])
			{
				case '0' : // Recevoir pseudo d'un joueur
				if(nbJoueurs < 4)
				{
					printf("message 0\n");
					char * chaine = malloc(1 + MAX_PSEUDO_LENGTH + 1);
					* chaine = (char)(nbJoueurs + 48); // + 48 pour avoir le caractères ASCII
					strncpy(chaine, buffer + 1, min(MAX_PSEUDO_LENGTH, strlen(buffer + 1)));
					joueurs[nbJoueurs] = JoueurDepuisChaine(chaine);

					char * msg = malloc(1 + sizeof(chaine));
					* msg = '0';
					* (msg + 1) = nbJoueurs;
					write(nouv_socket_descriptor, msg, strlen(msg) + 1);

					nbJoueurs++;
				}
				break;
				case '1' : // Récupérer ID du joueur
				break;
				case '2' : // Démarrer partie
				break;
			}
		}
		//close(nouv_socket_descriptor);	  
	}



}

void * ThreadJoueur(threadArgs * args)
{
	char buffer[256];

	while(* (* args).state == WaitPlayers)
	{
		if(read((* args).fd, buffer, sizeof(buffer)) > 0)
		{
			switch(buffer[0])
			{
				case '0':

				break;
				case '1':

				break;
			}
		}
	}
}
