#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>

/* Programme client */

int main(int argc, char *argv[]) {

 
  if (argc != 5){
    printf("utilisation : %s ip_serveur port_serveur \n", argv[0]);
    exit(1);
  }

/************************************************************/
/*  connection to main server                       */
/**************************************************************/
  int ds = socket(PF_INET, SOCK_STREAM, 0);

  if (ds == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }

  printf("Client : creation de la socket pour se conncter au main serveur Ã©ussie \n");
  

 
struct sockaddr_in MainServerInfo ;
MainServerInfo.sin_family = AF_INET ;
inet_pton(AF_INET,argv[1],&(MainServerInfo.sin_addr)) ;
MainServerInfo.sin_port = htons(atoi(argv[2]));



socklen_t lgA = sizeof(struct sockaddr_in) ;

int cnt=connect(ds, (struct sockaddr *)&MainServerInfo, lgA) ;
 if ( cnt == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }
 printf("Client : demande de connection au main serveur Ã©ussie \n");

/************************************************************/
/*  connection to an other process                         */
/**************************************************************/
int ds2 = socket(PF_INET, SOCK_STREAM, 0);
  if (ds2 == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }
  printf("Client : creation de la socket pour se connecter a un processuse rÃ©ussie \n");
struct sockaddr_in ServerInfo ;
ServerInfo.sin_family = AF_INET ;
inet_pton(AF_INET,argv[3],&(ServerInfo.sin_addr)) ;
ServerInfo.sin_port = htons(atoi(argv[4]));

int cnt2=connect(ds2, (struct sockaddr *)&ServerInfo , lgA) ;
 if ( cnt2 == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }

/************************************************************/
/*  change your state and send to main server               */
/**************************************************************/

int m=0;
int etat;
int rcv;
int snd;
while(1){ 



/*to process*/
 snd=send(ds2, &m, sizeof(int), 0) ;
if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }

  if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
  }

  etat=0;
  sleep(2);

  /*to main server*/
 snd=send(ds, &etat , sizeof(int), 0) ;

if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }

  if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
  }


  /*from process*/
  rcv = recv(ds2, &m, sizeof(int), 0);
if ( rcv == -1){
    perror("Client : pb de recv de message :");
    exit(1); 
  }

  if ( rcv == 0){
    perror("Client : client is closed :");
    exit(1); 
  }


  etat=1;
  sleep(2);

  /*to main server*/
 snd=send(ds, &etat , sizeof(int), 0) ;

if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }

  if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
  }




 

 printf("Client : j'attend pendant 3s\n");
 
}





int cls=close (ds) ;
if ( cls == -1){
    perror("Client : pb de fermeture de la socket :");
    exit(1); 
  }

int cls2=close (ds2) ;
if ( cls2 == -1){
    perror("Client : pb de fermeture de la socket :");
    exit(1); 
  }
  
  printf("Client : je termine\n");
  return 0;
}

