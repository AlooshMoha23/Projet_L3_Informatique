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

 
  if (argc != 3){
    printf("utilisation : %s ip_serveur port_serveur \n", argv[0]);
    exit(1);
  }


  int ds = socket(PF_INET, SOCK_STREAM, 0);

  if (ds == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }

  printf("Client : creation de la socket rÃ©ussie \n");
  

 
struct sockaddr_in ServerInfo ;
ServerInfo.sin_family = AF_INET ;
inet_pton(AF_INET,argv[1],&(ServerInfo.sin_addr)) ;
ServerInfo.sin_port = htons(atoi(argv[2]));



socklen_t lgA = sizeof(struct sockaddr_in) ;

int cnt=connect(ds, (struct sockaddr *)&ServerInfo, lgA) ;
 if ( cnt == -1){
    perror("Client : pb de connexion :");
    exit(1); 
  }

int m=1;
while(1){ 



int snd=send(ds, &m, sizeof(int), 0) ;

if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    exit(1); 
  }

  if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
  }


  if(m==1){

    m=0;
  }
  else{
    m=1;
  }

 printf("Client : j'attend pendant 3s\n");
 sleep(3);
}





int cls=close (ds) ;
if ( cls == -1){
    perror("Client : pb de fermeture de la socket :");
    exit(1); 
  }
  
  printf("Client : je termine\n");
  return 0;
}

