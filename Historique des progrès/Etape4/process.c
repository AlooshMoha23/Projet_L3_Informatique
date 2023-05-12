#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>

/* Programme client */

int rcv, snd;

int main(int argc, char *argv[]) {

 
  if (argc != 6){
    printf("utilisation : %s yourPort ip_MainServeur port_MainServeur ip_Server port_Server\n", argv[0]);
    exit(1);
  }

/************************************************************/
/*  connection to main server                       */
/**************************************************************/
  int MainserverSocket = socket(PF_INET, SOCK_STREAM, 0);

  if (MainserverSocket == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }

  printf("Client : creation de la socket pour se conncter au main serveur Ã©ussie \n");
  

 
struct sockaddr_in MainServerInfo ;
MainServerInfo.sin_family = AF_INET ;
inet_pton(AF_INET,argv[1],&(MainServerInfo.sin_addr)) ;
MainServerInfo.sin_port = htons(atoi(argv[2]));



socklen_t lgA = sizeof(struct sockaddr_in) ;

int cnt=connect(MainserverSocket, (struct sockaddr *)&MainServerInfo, lgA) ;
 if ( cnt == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }
 printf("Client : demande de connection au main serveur Ã©ussie \n");



/************************************************************/
/*  recv your nombre from main server       */
/**************************************************************/

int processNomber;

  rcv = recv(MainserverSocket, &processNomber, sizeof(int), 0);
if ( rcv == -1){
    perror("Client : pb de recv de message :");
    exit(1); 
  }

  if ( rcv == 0){
    perror("Client : client is closed :");
    exit(1); 
  }










 /************************************************************/
/*  accepting from client               */
/**************************************************************/


 //You can't listen and connect on the same socket
 int ServerSocket = socket(PF_INET, SOCK_STREAM, 0);


  if (ServerSocket == -1){
    perror("Serveur : pb creation socket :");
    exit(1); 
  }
  
 
  printf("Serveur : creation de la socket réussie \n");
  
  

  struct sockaddr_in  ad ;
  ad.sin_family = AF_INET ;
  ad.sin_addr.s_addr = INADDR_ANY;
  ad.sin_port = htons((short) atoi(argv[1])) ; 

  int res=bind(ds, (struct sockaddr*)&ad, sizeof(ad));
   if (res == -1){
    perror("Serveur : pb creation socket :");
    exit(1);
   }

  int lsn=listen(ServerSocket, 7);
   if ( lsn == -1){
    perror("Serveur : pb d'ecoute:");
    exit(1); 
  }
  printf("Serveur en Ecoute \n");  

/******************/
/*send socket decoute au main server*/
/*********/
snd=send(ds, &ad , sizeof(sockaddr_in), 0) ;

if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }

  if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
  }

/******************/



  struct sockaddr_in aC ;
 
  socklen_t lg = sizeof(struct sockaddr_in) ;
  

  int dSC= accept(ServerSocket, (struct sockaddr*) &aC,&lg) ;
    if ( dSC == -1){
    perror("Serveur : pb de connexion au client:");
    exit(1); 
  }




/************************************************************/
/*  recv struct of a server  from main server      */
/**************************************************************/

struct sockaddr_in ServerInfo ;
if(processNomber !=)
  rcv = recv(MainserverSocket, &ServerInfo, sizeof(sockaddr_in), 0);
if ( rcv == -1){
    perror("Client : pb de recv de message :");
    exit(1); 
  }

  if ( rcv == 0){
    perror("Client : client is closed :");
    exit(1); 
  }

/************************************************************/
/*  connect to an other process                         */
/**************************************************************/
int ClientSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (ds2 == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }
  printf("Client : creation de la socket pour se connecter a un processuse rÃ©ussie \n");
/*struct sockaddr_in ServerInfo ;
ServerInfo.sin_family = AF_INET ;
inet_pton(AF_INET,argv[3],&(ServerInfo.sin_addr)) ;
ServerInfo.sin_port = htons(atoi(argv[4]));*/

int cnt2=connect(ClientSocket, (struct sockaddr *)&ServerInfo , lgA) ;
 if ( cnt2 == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }



/************************************************************/
/*  exchanging with server and main Server                   */
/**************************************************************/