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

#include "course.c" // Divers types et fonctions en relation avec les entités du jeu de course
#include "utils.c"

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
    char * pseudo;


    if (argc != 2) {
        perror("usage : client <adresse-serveur>");
        exit(1);
    }
   
    prog = argv[0];
    host = argv[1];
    
    printf("nom de l'executable : %s \n", prog);
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
            pseudo = SaisirString(MAX_PSEUDO_LENGTH);
            char message[sizeof("0") + MAX_PSEUDO_LENGTH + 1] = "0";

            strcpy(message + 1, pseudo);

            int err;
            if((err = write(socket_descriptor, message, strlen(message) + 1)) != strlen(message) + 1)
                printf("Une erreur de communication est survenue : %d", err);
            else
                state = WaitForStart;
        } while (state != WaitForStart);
    }
    
    
    /**********************
    *  BOUCLE PRINCIPALE  *
    **********************/


   while(state == WaitForStart)
   {
        if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0)
        {
            printf("reponse du serveur : \n");
            write(1,buffer,longueur);
        }
    }


    printf("message envoye au serveur. \n");
                
    /* lecture de la reponse en provenance du serveur */
    while((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
	printf("reponse du serveur : \n");
	write(1,buffer,longueur);
    }
    
    printf("\nfin de la reception.\n");
    
    close(socket_descriptor);
    
    printf("connexion avec le serveur fermee, fin du programme.\n");
    
    exit(0);
}
