/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  

#define TAILLE_MAX_NOM 256

typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

struct tJoueur
{
    int id;
    int avancee;
} typedef joueur;

enum ServerState
{
    Init,
    WaitPlayers,
    Starting,
    Playing,
    Paused,
    Finished
};


/*------------------------------------------------------*/
void gererMessage(int sock)
{
    char buffer[256];
    int longueur;
    buffer[1] = 'p';
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
    	return;
    
    printf("premier char : %c", buffer[0]);
    printf("message lu : %s \n", buffer);

    switch(buffer[0])
    {
      case '0':
        printf("%s", "message 0");
        break;
      case '1':
        printf("%s", "message 1");
        break;
      case '2':
        printf("%s", "message 2");
        break;      
      case '3':
        printf("%s", "message 3");
        break;          
    }
        
    write(sock,buffer,strlen(buffer)+1);
    
    printf("message envoye. \n");
        
    return;
}


main(int argc, char **argv) {
    int port = 12345;
    int socket_descriptor, 		/* descripteur de socket */
        nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
        longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in adresse_locale, 		/* structure d'adresse locale*/
        adresse_client_courant; 	/* adresse client courant */
    hostent* ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent* ptr_service; 			/* les infos recuperees sur le service de la machine */
    char machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */



    joueur joueurs[4]; /* Tableau contenant les joueurs */
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

    /* attente des connexions et traitement des donnees recues */
    while(1)
    {    
        longueur_adresse_courante = sizeof(adresse_client_courant);
        /* adresse_client_courant sera renseigné par accept via les infos du connect */
        if ((nouv_socket_descriptor = accept(socket_descriptor, (sockaddr*)(&adresse_client_courant), &longueur_adresse_courante)) < 0)
        {
            perror("erreur : impossible d'accepter la connexion avec le client.");
            exit(1);
        }
        gererMessage(nouv_socket_descriptor);

        close(nouv_socket_descriptor);      
    }
}
