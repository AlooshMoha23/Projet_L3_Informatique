#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>

/* Programme serveur */

int main(int argc, char *argv[]) {

if (argc != 2){
    printf("utilisation : %s port_serveur\n", argv[0]);
    exit(1);
  }
int ds1 = socket(PF_INET, SOCK_STREAM, 0);

  if (ds1 == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }

  printf("Client : creation de la socket réussie \n");
  

 
struct sockaddr_in aS ;
aS.sin_family = AF_INET ;
inet_pton(AF_INET,"127.0.0.1",&(aS.sin_addr)) ;
aS.sin_port = htons(3000) ;



socklen_t lgA = sizeof(struct sockaddr_in) ;

int cnt=connect(ds1, (struct sockaddr *) &aS, lgA) ;
 if ( cnt == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }


 int m=1;
while(1){
int snd=send(ds1, &m, sizeof(int), 0) ;
if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }
  if(snd==0){
    perror("client: Socket fermé");
    exit(1);
  }
  if(m==1){
    m=0;
  }else{
    m=1;
  }
 
sleep(2);

} 
 

 
  int ds = socket(PF_INET, SOCK_STREAM, 0);


  if (ds == -1){
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

  int lsn=listen(ds, 7);
   if ( lsn == -1){
    perror("Serveur : pb d'ecoute:");
    exit(1); 
  }
  printf("Serveur en Ecoute \n");
  struct sockaddr_in aC ;
 
  socklen_t lg = sizeof(struct sockaddr_in) ;
  

  int dSC= accept(ds, (struct sockaddr*) &aC,&lg) ;
    if ( dSC == -1){
    perror("Serveur : pb de connexion au client:");
    exit(1); 
  }
}