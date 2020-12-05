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
#include <unistd.h>

#include "libs/course.c" // Divers types et fonctions en relation avec les entités du jeu de course

#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;


struct TserverThreadArgs {
	enum State * state;
	joueur * j;
	int monId;
	int fd;
	int * sockets;
} typedef serverThreadArgs;

void GlobalMessage(char * message, int * sockets);
void GlobalMessageSize(char * message, int * sockets, int size);
void InitJoueurs(joueur joueurs[MAX_NB_JOUEURS]);
void * ThreadJoueur(void * arg);
int main(int argc, char **argv);


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

	serverThreadArgs threadArgs[MAX_NB_JOUEURS];
	pthread_t threads[MAX_NB_JOUEURS];
	int clientSockets[MAX_NB_JOUEURS]; //Contient tous les sockets des clients, pour par exemple envoyer des messages globaux

	/* Variables liées au jeu */
	joueur joueurs[MAX_NB_JOUEURS]; /* Tableau contenant les joueurs */
	int nbJoueurs = 0;

	enum State state = Init;

	
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

	// Côté Serveur : 
	//   0 = Le client envoie ses informations (pseudo, IP)
	//   1 = Le client hôte envoie le signal "Démarrage de la partie"
	//   2 = Le client envoie le signal "Appui sur Espace"
	//   3 = Le client envoie le signal "Pause"
	//   4 = Le client envoie le signal "Joueur Gagnant"
	

	InitJoueurs(joueurs);


	state = WaitForStart;
	while(state == WaitForStart)
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
			// Si le nombre de joueurs n'est pas encore atteint, on peut créer un nouveau thread pour la nouvelle connexion
			if(nbJoueurs < MAX_NB_JOUEURS)
			{
				clientSockets[nbJoueurs] = nouv_socket_descriptor;

				threadArgs[nbJoueurs] = (serverThreadArgs)
				{
					&state, joueurs, nbJoueurs, nouv_socket_descriptor, &clientSockets
				};
				pthread_create(&(threads[nbJoueurs]), NULL, &ThreadJoueur, &threadArgs[nbJoueurs]);
				nbJoueurs++;
				printf("Nombre de joueurs : %d, dernier arrivé a le thread : %d\n", nbJoueurs, nouv_socket_descriptor);
			}
		}
	}
}


void * ThreadJoueur(void * arg)
{
	serverThreadArgs * args = arg;
	printf("init : thread %d\n", (* args).fd);
	printf("Id joueur thread : %d\n", (* (* args).j).id);
	char buffer[256];

	while(* (* args).state != Finished)
	{
		if(read((* args).fd, buffer, sizeof(buffer)) > 0)
		{
			printf("message reçu par le thread %d : '%s'\n", (* args).fd, buffer);
			switch(buffer[0])
			{
				case '0': // Pseudo envoyé
					;
					char chaineJoueur[1 + MAX_PSEUDO_LENGTH + 1];
					// ID du joueur en premier caractère de la chaine
					* chaineJoueur = (char)((* args).monId + 48); // + 48 pour avoir le caractères ASCII
					// Pseudo du joueur, dans le buffer du socket, pour le reste de la chaine
					strncpy(chaineJoueur + 1, buffer + 1, strlen(buffer + 1) + 1);

					// Transformation de la chaine en struct de type joueur
					(* args).j[(* args).monId] = JoueurDepuisChaine(chaineJoueur);

					// Mise en forme du message à envoyer au joueur géré par ce thread
					char msgIdJoueur[1 + sizeof(2) + 1];
					* msgIdJoueur = '1';
					* (msgIdJoueur + 1) = (char)((* args).j[(* args).monId].id + 48);

					// Mise en forme du message à envoyer à tous les autres joueurs pour les alerter d'un nouveau joueur.
					char msgInfosJoueur[1 + sizeof(chaineJoueur) + 1];
					* msgInfosJoueur = '0'; // 0 est le code pour un nouveau joueur, côté client
					strncpy(msgInfosJoueur + 1, chaineJoueur, sizeof(chaineJoueur) + 1);
					
					// Renvoyer au joueur responsable du message son ID
					write((* args).fd, msgIdJoueur, strlen(msgIdJoueur) + 1);

					for(int i = 0; i < MAX_NB_JOUEURS; i++)
					{
						// Pour chaque socket,
						// Si le socket actuellement itéré est différent du socket client géré par le thread,
						// Et que le socket itéré n'est pas nul,
						// Et que le client utilisant ce socket s'est bien connecté au serveur en envoyant son pseudo,
						if((* args).sockets[i] != (* args).fd && (* args).sockets[i] != 0 && (* args).j[i].pseudo[0] != '\0')
						{
							// Alors on envoie au socket itéré les infos du joueur à l'origine du message,
							write((* args).sockets[i], msgInfosJoueur, strlen(msgInfosJoueur) + 1);		
							printf("> '%s' Ecrit dans %d (moi %d : %d)\n", msgInfosJoueur, (* args).sockets[i], (* args).fd, ((* args).sockets[i] != (* args).fd));

							// Et on envoie au socket à l'origine du message les infos du client du socket itéré.
							char jAEnvoyer[1 + MAX_PSEUDO_LENGTH + 1];
							char message[1 + strlen(jAEnvoyer) + 1];
				
							ChaineDepuisJoueur(&(* args).j[i], jAEnvoyer);
							* message = '0';
							strncpy(message + 1, jAEnvoyer, strlen(jAEnvoyer) + 1);

							write((* args).fd, message, strlen(message) + 1);
							printf("> Joueur '%s' envoyé au socket %d\n", message, (* args).fd);
						}
						printf("\n");
					}					
				break;
				case '1':; // Démarrer la partie
				printf("> Démarrage de la partie.\n");
				
				GlobalMessage("2d", (* args).sockets);

				printf("\n");
				break;

				case '2':; // Le joueur appuie sur un bouton lors du jeu
					(* args).j[(* args).monId].avancee++;
					printf("Joueur %d a avancé et est à %d.\n", (* args).monId, (* args).j[(* args).monId].avancee);
					if((* args).j[(* args).monId].avancee >= SCORE)
					{
						printf("> Joueur %d a gagné !\n", (* args).monId);
						char message[3 + 1] = "5_g";
						message[1] = (* args).monId + 48;
						GlobalMessage(message, (* args).sockets);
					}
					else
					{
						char message[1 + (1 + 1) * MAX_NB_JOUEURS + 1];
						message[0] = '3';
						message[strlen(message)] = '\0';

						int nb = 0;
						for(int i = 0; i < MAX_NB_JOUEURS; i++)
						{
							if((* args).j[i].pseudo[0] != '\0')
							{
								printf("%d\n", nb);
								message[1 + 2 * nb] = (* args).j[i].id + 48;
								message[1 + 2 * nb + 1] = (char)(* args).j[i].avancee; // Le caractère peut être à \0, mais le client interprètera ça comme un chiffre.
								
								nb++;
							}
						}
						printf("Message : %s\n", message);
						PrintAscii(message, 10);
						GlobalMessageSize(message, (* args).sockets, (1 + (1 + 1) * MAX_NB_JOUEURS));
					}					
				break;			
			}
			printf("\n");
		}
	}
	return NULL;
}


/// Envoie un message à tous les FD de sockets qui ne sont pas nuls.
void GlobalMessage(char * message, int * sockets)
{
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
		// Pour chaque socket non nul,
		if(sockets[i] != 0)
		{
			// Envoyer le message permettant de signaler le début de la partie à chaque socket.
			write(sockets[i], message, strlen(message) + 1);
		}
	}
}


/// Envoie un message à tous les FD de sockets qui ne sont pas nuls, avec une taille donnée en paramètre.
void GlobalMessageSize(char * message, int * sockets, int size)
{
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
		// Pour chaque socket non nul,
		if(sockets[i] != 0)
		{
			// Envoyer le message permettant de signaler le début de la partie à chaque socket.
			write(sockets[i], message, size + 1);
		}
	}
}