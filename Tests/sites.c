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

  //se connecter a orch

  if (argc != 10){
    printf("utilisation : %s Ip_configuration, Port_configuration, IP_SocketEcoute,  Port_SocketEcoute, IP_MainServerSocket, Port_MainServerSocket, IP_To_MainServeur, Port_To_MainServeur,  MonID\n", argv[0]);
    exit(1);
  }

  int IdSite=atoi(argv[9]);

    int MainServerSocket = socket(PF_INET, SOCK_STREAM, 0);

      if (MainServerSocket == -1){
    perror("serveur : pb creation MainServeur Socket :");
    exit(1); 
  }

  struct sockaddr_in  MainServerStruct;
  MainServerStruct.sin_family = AF_INET ;
    inet_pton(AF_INET,argv[7],&(MainServerStruct.sin_addr)) ;
  MainServerStruct.sin_port = htons((short) atoi(argv[8])) ; 

   int res=bind(MainServerSocket, (struct sockaddr*)&MainServerStruct, sizeof(MainServerStruct));
   if (res == -1){
    perror("site : pb bind MainServerSocket  :");
    close(MainServerSocket);
    exit(1);
   } 







  //se connecter au main serveur
  struct  sockaddr_in adrMainServer ;
  adrMainServer.sin_family=AF_INET;
  adrMainServer.sin_addr.s_addr=inet_addr(argv[5]);
  adrMainServer.sin_port=htons((short) atoi(argv[6])) ; 
  printf("ip main server %s....port %s\n", argv[5], argv[6]);

  //connexion
  if(connect(MainServerSocket,(struct sockaddr *)&adrMainServer,sizeof(struct sockaddr_in ))==-1){
      perror("probleme de connexions au main serveur");
      close(MainServerSocket);
      exit(1);
      }

     int snd=send(MainServerSocket, &IdSite , sizeof(int), 0) ;

    if ( snd == -1){
    perror("Client : pb d'envoi de struct :");
    exit(1); 
    }

    if ( snd == 0){
    perror("Client : server is closed :");
    exit(1); 
   }
  printf(" connexion to main server with id  : %d\n", IdSite);


  /* Etape 1 : créer une socket */   
  int dsOrch = socket(PF_INET, SOCK_STREAM, 0);
  
  if (dsOrch == -1){
    perror("Client : pb creation socket :");
    exit(1); 
  }
  

  printf("Client : creation de la socket de l'orch réussie \n");
  
  
  
  /* Etape 3 : Désigner la socket du orchestateur */
  struct  sockaddr_in adrOrch;
  adrOrch.sin_family=AF_INET;
  adrOrch.sin_addr.s_addr=inet_addr(argv[1]);
  adrOrch.sin_port=htons((short) atoi(argv[2]));

  socklen_t lgAdr=sizeof(struct sockaddr_in);
  printf("ip orch= %s..port orch= %s", argv[1],argv[2]);

  //demande de la connexion
  int conn=connect(dsOrch,(struct sockaddr *)&adrOrch,lgAdr);
  if(conn<0){
   perror("Client:peobleme de la  connexion à l'orch \n");
  }
  printf("Client:connexion réussi a l'orch");
  
  /**********************************/

//creer un socket d'ecoute 
  int ds = socket(PF_INET, SOCK_STREAM, 0);

  
  if (ds == -1){
    perror("serveur : pb creation socket :");
    exit(1); 
  }
  
  
  printf("id %d, serveur : creation de la socket d'ecoute réussie \n", IdSite);
  
  struct  sockaddr_in sockEcoute ;
  sockEcoute.sin_family=AF_INET;
  inet_pton(AF_INET,argv[3],&(sockEcoute.sin_addr)) ;
  sockEcoute.sin_port=htons((short) atoi(argv[4]));

  int bd=bind(ds,(struct sockaddr*)&sockEcoute,sizeof(sockEcoute));
 //test de bind
 if(bd <0){
    perror(" Serveur:probleme de nommage");
  }
  printf("id %d, site: nommge de la socket d'ecoute reussit!\n", IdSite);
  int l = listen(ds,256);
  if(l<0){
    printf("Serveur:echec en mode ecoute");
    exit(1);
  }
  printf("id %d, Serveur:mode ecoute réussi\n", IdSite);  
  
  //envoyer l'id et les infos de la socket d'ecoute a orch
 
  snd=send(dsOrch,&IdSite,sizeof(int),0);
  if (snd == -1){
    perror("pb d'envoyer les infos de sock ecoute a l'orch");
    exit(1); 
  }
   printf("id %d, id sent to orch!\n", IdSite);
  
  snd=send(dsOrch,&sockEcoute,sizeof(sockEcoute),0);
  if (snd == -1){
    perror("pb d'envoyer les infos de sock ecoute a l'orch");
    exit(1); 
  }
  
  printf("id %d, struct bien envoyé!\n", IdSite);

  //recevoir un tableau de struct
  int nbSites;
  

  
  int rec=recv(dsOrch,&nbSites,sizeof(int),0); 
  if (rec == -1){
    perror("pb de recevoir le struc");
    exit(1); 
  }
  
  printf("id %d, nbsite recved from orch is %d\n",IdSite, nbSites);

  struct sockaddr_in sitesInfos[nbSites+1];
   struct sockaddr_in SendToMainServer[nbSites];

   

  
  rec=recv(dsOrch,sitesInfos,sizeof(sitesInfos),0); 
  if (rec == -1){
    perror("pb de recevoir le struc");
    exit(1); 
  }
  
  printf("id %d, struct bien recu!\n", IdSite);


  
 
 





  //creat a socket to connet to sites 
  int connectionSocket = socket(PF_INET, SOCK_STREAM, 0);

    
  if (connectionSocket == -1){
    perror("serveur : pb creation socket :");
    exit(1); 
  }




if(nbSites==1){

  if(connect(connectionSocket,(struct sockaddr *)&sitesInfos[0],sizeof(struct sockaddr_in ))==-1){
      perror("probleme de connexions aux sites");
      }
       printf("id %d, demande conection sent to site 0 qui a ....ip= %s....port= %d \n", IdSite, inet_ntoa(sitesInfos[0].sin_addr),ntohs(sitesInfos[0].sin_port));


struct sockaddr_in clientInfo;
    socklen_t lg=sizeof(struct sockaddr_in);
    int accpt=accept(ds,(struct sockaddr*)&clientInfo,&lg);
    if(accpt==-1){
      perror("echec de l'acceptation");
      exit(1);
    }
     printf("conection accepted from site 0 qui a ......ip= %s....port= %d \n", inet_ntoa(clientInfo.sin_addr),ntohs(clientInfo.sin_port));

      printf("id %d, conection accepted \n", IdSite);
      

    //send your mainserveur socket to this client 
     snd=send(accpt,&MainServerStruct,sizeof(MainServerStruct),0);
     if (snd == -1){
        perror("pb d'envoyer les infos de sock ecoute a l'orch");
        exit(1); 
    }
    if ( snd == 0){
                     printf("Serveur :Socket est ferme");
                       exit(1);
                      }

      printf("id %d, struct of main server sent to client\n", IdSite);

     //recv socket socket to this client 
     int rcv=recv(connectionSocket, &MainServerStruct, sizeof(MainServerStruct), 0);
                if ( rcv == -1){
                     perror("Serveur : pb de reception");
                        exit(1);
                      }
                 if ( rcv == 0){
                     printf("Serveur :Socket est ferme");
                       exit(1);
                      }

      printf("id %d, struct recved from server is....ip= %s....port= %d \n",IdSite, inet_ntoa(MainServerStruct.sin_addr),ntohs(MainServerStruct.sin_port));
                      
      memcpy(&SendToMainServer[0], &MainServerStruct, sizeof(struct sockaddr_in)); 

      
}






if(nbSites > 1){ 

  int SendDiscripteurs[nbSites+1];

  for (int i = 0; i <=nbSites; i++) {
    SendDiscripteurs[i] = 0;
}
  
  

 
  for(int i=1;i<=nbSites;i++){

   


  if (memcmp(&sitesInfos[i].sin_addr,&sockEcoute.sin_addr, sizeof(sockEcoute.sin_addr) )!= 0 || 
      memcmp(&sitesInfos[i].sin_port,&sockEcoute.sin_port, sizeof(sockEcoute.sin_port) )!= 0 ) {
        
    SendDiscripteurs[i]= connectionSocket;
    if(connect(connectionSocket,(struct sockaddr *)&sitesInfos[i],sizeof(struct sockaddr_in ))==-1){
      perror("probleme de connexions aux sites");
      }
    
    printf("id %d, demande envoyé au site qui a...ip= %s...port= %d \n" ,IdSite, inet_ntoa(sitesInfos[i].sin_addr),ntohs(sitesInfos[i].sin_port));
    printf("discrpteur send = %d\n" , SendDiscripteurs[i]);

    connectionSocket = socket(PF_INET, SOCK_STREAM, 0);
                                                      } 
  }


for(int i=1;i<nbSites;i++){
    //puis accepter des connexion avec sock ecoute
    struct sockaddr_in clientInfo;
    socklen_t lg=sizeof(struct sockaddr_in);
     printf("id = %d, waitting to accept connections from other sites\n", IdSite ); 
    int accpt=accept(ds,(struct sockaddr*)&clientInfo,&lg);
    if(accpt==-1){
      perror("echec de l'acceptation");
      exit(1);
    }
    printf("id %d, connection accepté de site qui a....ip= %s....port= %d \n",IdSite, inet_ntoa(clientInfo.sin_addr),ntohs(clientInfo.sin_port));

     


    snd=send(accpt ,&MainServerStruct,sizeof(MainServerStruct),0);
     if (snd == -1){
        perror("pb d'envoyer les infos de sock ecoute a l'orch");
        exit(1); 
    }
    if ( snd == 0){
                     printf("Serveur :Socket est ferme");
                       exit(1);
                      }

      printf("id %d, struct of main server sent to client\n", IdSite);


}



 int n=0;
for(int i=0;i<=nbSites;i++){

   
 

  if(SendDiscripteurs[i] != 0){
    printf("id %d, j'attend une connection sur la socket %d\n", IdSite,SendDiscripteurs[i]);

     //recv socket socket to this client 
     int rcv=recv(SendDiscripteurs[i] , &MainServerStruct, sizeof(MainServerStruct), 0);
                if ( rcv == -1){
                     perror("Serveur : pb de reception");
                        exit(1);
                      }
                 if ( rcv == 0){
                     printf("Serveur :Socket est ferme");
                       exit(1);
                      }

      printf("id %d,  struct recved from server is.....ip= %s....port= %d \n",i, inet_ntoa(MainServerStruct.sin_addr),ntohs(MainServerStruct.sin_port));
                      
      memcpy(&SendToMainServer[n], &MainServerStruct, sizeof(struct sockaddr_in)); 

      printf("id %d,  comparer....ip= %s....port= %d \n",n, inet_ntoa(SendToMainServer[n].sin_addr),ntohs(SendToMainServer[n].sin_port));
      n+=1;

  } 

}

} 
  




  
   close(dsOrch);



 //echange avec le main serveur

if(nbSites > 1){



for(int i=0;i<nbSites-1;i++){
  snd=send(MainServerSocket, &SendToMainServer[i] , sizeof(struct sockaddr_in), 0) ;

    if ( snd == -1){
    perror("Client : pb d'envoi de struct :");
    close(MainServerSocket);
    exit(1); 
    }

    if ( snd == 0){
    perror("Client : server is closed :");
    close(MainServerSocket);
    exit(1); 
   }

  
  printf("id %d, struct sent to main server is....ip= %s....port= %d \n", IdSite, inet_ntoa(SendToMainServer[i].sin_addr),ntohs(SendToMainServer[i].sin_port));

}
}



if(nbSites = 1){

  snd=send(MainServerSocket, &SendToMainServer[0] , sizeof(struct sockaddr_in), 0) ;

    if ( snd == -1){
    perror("Client : pb d'envoi de struct :");
    close(MainServerSocket);
    exit(1); 
    }

    if ( snd == 0){
    perror("Client : server is closed :");
    close(MainServerSocket);
    exit(1); 
   }

  printf("id %d, struct sent to main server is....ip= %s...port= %d \n", IdSite, inet_ntoa(SendToMainServer[0].sin_addr),ntohs(SendToMainServer[0].sin_port));

}

int m=1;
if(IdSite==1){
  m=4;
}
//while(1){
 snd=send(MainServerSocket, &m, sizeof(int), 0) ;
if ( snd == -1){
    perror("Client : pb d'envoi de message :");
    close(MainServerSocket);
    exit(1); 
  }
  if(snd==0){
    perror("client: Socket fermé");
    close(MainServerSocket);
    exit(1);
  }
  printf("id %d, Msg : %d\n",IdSite, m);
  if(m==1){
    m=2;
  }
    else if(m==2){
    m=3;
  }
  else if(m==3){
    m=4;
  }else{
    m=1;
  }
 
sleep(20);

//}


close(MainServerSocket);






return 0;


  } 



  

  
  







