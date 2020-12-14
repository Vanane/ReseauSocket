/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

#include "libs/conio.c"
#include "libs/course.c" // Divers types et fonctions en relation avec les entités du jeu de course

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


struct TclientThreadArgs {
	enum State * state;
	joueur * joueurs;
	int fd;
	int * monId;
	char * pseudo;
	int * pauseId;
} typedef clientThreadArgs;


// GLOBALS
struct winsize WINSIZE; // Récupère les informations sur la fenêtre de commande.

void InitScreen(joueur * joueurs);
void UpdateScreen(joueur * joueurs);
void RecupererId(char * message, int * monId);
void InfoJoueur(char * message, joueur joueurs[]);
void * ThreadJoueur(void * arg);
void DemarrerPartie(enum State s);
void PositionJoueurs(char * message, joueur joueurs[]);
void PauserJeu(enum State s);
void AfficheEcranFin(char * pseudo);
void JoueurGagnant(char * message, enum State s);
int main(int argc, char **argv);


int main(int argc, char **argv) {
	/* Variables liées au socket */
	int port = 12345;
	int 	socket_descriptor;	/* descripteur de socket */
	sockaddr_in adresse_locale; 	/* adresse de socket local */
	hostent *	ptr_host; 		/* info sur une machine hote */
	char 	buffer[256];
	char *	host; 			/* nom de la machine distante */

	pthread_t thread;
	int quitter = 0;

	/* Variables liées au jeu */
	enum State state = Init;

	int monId;
	joueur joueurs[MAX_NB_JOUEURS];
	char pseudo[MAX_PSEUDO_LENGTH];


	ioctl(STDOUT_FILENO, TIOCGWINSZ, &WINSIZE);


	if (argc != 2) {
		perror("usage : client <adresse-serveur>");
		exit(1);
	}
   
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
	while(quitter == 0)
	{
		printf("Voulez-vous (r)ejoindre une partie en cours ou en (c)réer une nouvelle ? R ou C, Q pour quitter.\n");
		char choix = (char) fgetc(stdin);
		if(choix == 'q' || choix =='Q')
			exit(0);
			
		if(choix == 'c' || choix =='C')
		{
			char * message = "8";
			write(socket_descriptor, message, strlen(message) + 1);
			read(socket_descriptor, buffer, sizeof(buffer));
			if((int)buffer[1] != -1)
			{
				printf("Partie créée ! Numéro : %d\n", (int)buffer[1]);
			}
		}
		else
		{
			int salleExiste;
			do
			{
				salleExiste = 0;
				printf("Quel numéro de partie voulez-vous rejoindre ?\n");
				int num;
				scanf("%d", &num);
				char  message[2 + 1] = "9_";
				message[1] = (char) num;
				write(socket_descriptor, message, strlen(message) + 1);
				read(socket_descriptor, buffer, sizeof(buffer));
				if(buffer[0] == '9' && buffer[1] == '1')
				salleExiste = 1;
				else
					printf("Cette partie n'est pas disponible !\n");
			} while (salleExiste == 0);			
		}
		

		InitJoueurs(joueurs);	
		do
		{
			printf("Choisissez votre pseudo :\n");
			SaisirString(MAX_PSEUDO_LENGTH, pseudo);
			printf("Pseudo à envoyer :%s\n", pseudo);

			char message[1 + MAX_PSEUDO_LENGTH + 1];
			* message = '0';
			strcpy(message + 1, pseudo);

			clientThreadArgs args =
			{
				&state, joueurs, socket_descriptor, &monId, pseudo
			};
			pthread_create(&thread, NULL, &ThreadJoueur, &args);

			int err;
			if((err = write(socket_descriptor, message, strlen(message) + 1)) != strlen(message) + 1)
				printf("Une erreur de communication est survenue : %d\n", err);
			else
			{
				state = WaitForId;
			}

		} while (state != WaitForId);

		while(state == WaitForId);

		if(monId == 0)
		{
			printf("\n> Vous êtes l'hôte. Appuyez sur Entrée pour commencer la partie.\n\n");
			char saisie[2];
			fgets(saisie, 3, stdin);
			printf("\n> Démarrage de la partie.\n\n");
			char * message = "1d";
			write(socket_descriptor, message, strlen(message) + 1);
		}
		else
		{
			printf("En attente de démarrage...\n");
		}

		while(state == WaitForStart);
		
		printf("Go ! \n");


		// Boucle principale du jeu
		// Tant que ce n'est pas fini, on boucle sur l'état actuel.
		char * keys = "gd";
		int next = 0;

		InitScreen(joueurs);
		UpdateScreen(joueurs);
		
		while(state != Finished)
		{
			switch(state)
			{
				case Paused :; // Si un joueur appuie sur la touche de pause, on bascule en pause, et il ne se passe plus rien.
				break;
				case Started :; // La partie n'est pas en pause, le joueur peut appuyer sur les touches.
				if(c_kbhit())
				{
					char c = c_getch();
					if(c == (char) keys[next])
					{
						char message[3 + 1] = "2_e";
						message[1] = monId + 48;

						write(socket_descriptor, message, strlen(message) + 1);
						next = (next + 1) % strlen(keys);
						printf("\b\b\b%c !\n", (char) keys[next]);
						joueurs[monId].avancee++;
						UpdateScreen(joueurs);
					}
					else
					{
						if (c == 'p')
						{
							char message[2 + 1] = "3_";
							message[1] = monId + 48;
							write(socket_descriptor, message, strlen(message) + 1);
						}
						
					}
					
				}
				break;
				default:;
			}
		}
		printf("\n");
	}

	close(socket_descriptor);
	
	exit(0);
}


void InfoJoueur(char * message, joueur joueurs[])
{
	joueur nouvJ = JoueurDepuisChaine(message + 1);
	joueurs[nouvJ.id] = nouvJ;
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

void * ThreadJoueur(void * arg)
{
	clientThreadArgs * args = arg;
	int nbJoueurs = 0;
	char buffer[256];
	
	while(* (* args).state != Finished)
	{
		if(read((* args).fd, buffer, sizeof(buffer)) > 0)
		{
			joueur nJ;
			printf("> Message reçu : %s\n", buffer);
			switch(buffer[0])
			{
				case '0' : // Récupérer infos d'un joueur
					nJ = JoueurDepuisChaine(buffer + 1);
					printf("> Nouveau joueur :");
					printf("\tId : %d", nJ.id);
					printf("\tPseudo : %s\n", nJ.pseudo);
					(* args).joueurs[nJ.id] = nJ;
					nbJoueurs++;
				
					printf("> Joueurs connectés : ");
					for(int i = 0; i < nbJoueurs; i++)
					{
						printf("'%s'", (* args).joueurs[i].pseudo);
						if(i < nbJoueurs - 1)
							printf(", ");
					}
					printf(".\n");

				break;
				case '1' : // Récupérer ID du joueur
					RecupererId(buffer + 1, (* args).monId);
					(* args).joueurs[* (* args).monId].id = * (* args).monId;
					strncpy((* args).joueurs[* (* args).monId].pseudo, (* args).pseudo, strlen((* args).pseudo) + 1);					
					
					printf("> Mon Id est %d\n", * (* args).monId);
					nbJoueurs++;
					* (* args).state = WaitForStart;
				break;
				case '2' : // Démarrer partie
					* (* args).state = Started;
				break;
				case '3' : // Position des joueurs
					for(int i = 0; i < MAX_NB_JOUEURS; i++)
					{
						int idJ = (buffer + 1)[2 * i] - 48;
						int scoreJ = (buffer + 1)[2 * i + 1];
						(* args).joueurs[idJ].avancee = scoreJ;
					}
					UpdateScreen((* args).joueurs);
				break;
				case '4' : // Pause

				break;
				case '5' : // Vainqueur
					* (* args).state = Finished;
					int idJ;
					RecupererId(buffer + 1, &idJ);
					AfficheEcranFin((* args).joueurs[idJ].pseudo);
				break;
				default :
					printf("> Message reçu et non-traité : %s\n", buffer);
				break;
			}
		}
	}	
	return NULL;	
}


void AfficheEcranFin(char * pseudo)
{
	char * separateur = "* ";
	char * messageFin = " a gagné la partie ! ";
	printf("\n\n");

	for(int i = 0; i < (strlen(messageFin) + strlen(pseudo) + 3) / 2; i++)
	{
		printf("%s", separateur);
	}

	printf("\n");
	printf("* %s%s*\n", pseudo, messageFin);

	for(int i = 0; i < (strlen(messageFin) + strlen(pseudo) + 3) / 2; i++)
	{
		printf("%s", separateur);
	}
	printf("\n");
}


void InitScreen(joueur * joueurs)
{
	int x = WINSIZE.ws_col - 1;
	int y = WINSIZE.ws_row - 1;

	int paddingX = 2, paddingY = 2;
	int barPadding = 2;
	
	char * score = "Score à atteindre : ";

	c_clrscr();
	c_gotoxy(0, 0);
	printf("> Appuyez alternativement sur G et D pour avancer !");
	
	c_gotoxy(x - strlen(score) - 2 * paddingX, 0);
	printf("%s%d\n", score, SCORE);

	c_gotoxy(paddingX, paddingY);
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
		if(joueurs[i].pseudo[0] != '\0')
		{
			c_gotoxy(paddingX, paddingY + (barPadding + 1) * i);
			printf("%s :\n", joueurs[i].pseudo);
		}
	}
}



void UpdateScreen(joueur * joueurs)
{
	int paddingX = 2, paddingY = 2;
	int barPadding = 2;

	c_gotoxy(paddingX, paddingY);
	for(int i = 0; i < MAX_NB_JOUEURS; i++)
	{
		if(joueurs[i].pseudo[0] != '\0')
		{
			char bar[SCORE + 1];
			for(int j = 0; j < joueurs[i].avancee; j++)
			{
				bar[j] = '#';
			}
			bar[joueurs[i].avancee] = '\0';
			c_gotoxy(paddingX, paddingY + (barPadding + 1) * i + 1);
			printf("%s %d\n", bar, joueurs[i].avancee);
		}
	}
}