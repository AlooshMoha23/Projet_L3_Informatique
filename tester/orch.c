#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include <errno.h>
#include <fcntl.h>
#define TRUE             1
#define FALSE            0




int main(int argc, char *argv[]) {

 
  if (argc != 5){
    printf("utilisation : %s IP, port, choix (anneau->0, réseau complet->1),  nb_sites \n", argv[0]);
    exit(1);
  }


int ds = socket(PF_INET, SOCK_STREAM, 0);


  if (ds == -1){
    perror("Serveur : pb creation socket :");
    exit(1); 
  }
  
 
  printf("Serveur : creation de la socket reussie \n");

 

   /*************************************************************/
   /* Bind the socket                                           */
   /*************************************************************/
  struct sockaddr_in  ad;
  ad.sin_family = AF_INET ;
  inet_pton(AF_INET,argv[1],&(ad.sin_addr)) ;

  ad.sin_port = htons((short) atoi(argv[2])); 
  
  int res=bind(ds, (struct sockaddr*)&ad, sizeof(ad));
   if (res == -1){
    perror("Serveur : pb creation socket :");
    exit(1);
   } 

   int size = sizeof(ad); 
   getsockname(ds, (struct sockaddr *) &ad, &size);
   printf("Socket bound to IP: %s\n", inet_ntoa(ad.sin_addr));


/*---------------Set the listen fonction--------------------*/
   int nbsites=atoi(argv[4]);
   int choix=atoi(argv[3]);

   int lsn=listen(ds, nbsites);
   if ( lsn == -1){
    perror("Serveur : pb d'ecoute:");
    exit(1); 
  }



int rcv,snd;
socklen_t lg = sizeof(struct sockaddr_in) ;
struct sockaddr_in  aC;
struct sockaddr_in sites[nbsites+1];
struct sockaddr_in SendToSites[1];
int sockets[nbsites];
int ids[nbsites];



    /*------------------------ Allow socket descriptor to be reuseable--------------------*/
   int on = 1, rc, i;
   rc = setsockopt(ds, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(ds);
      exit(-1);
   }
  
   /*--------------------Set socket to be nonblocking.---------------*/

    int flags = fcntl(ds, F_GETFL, 0);
    fcntl(ds, F_SETFL, flags | O_NONBLOCK);

   /*----------------Initialize the master fd_set---------------------*/
   fd_set        master_set;
   fd_set        working_set;
   int     max_sd, new_sd;
   int compteur=0;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   FD_ZERO(&master_set);
   max_sd = ds;
   FD_SET(ds, &master_set);
   


   
   /* Loop waiting for incoming connects or for incoming data   */
   /* on any of the connected sockets.                          */
   
   do
   {
     
      /*------------------Copy the master fd_set over to the working fd_set.-------------*/
   
      memcpy(&working_set, &master_set, sizeof(master_set));

      
      /*-------------------------Call select()-----------------*/    
     
      printf("Waiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, NULL);

      /**********************************************************/
      /* Check to see if the select call failed.                */
      /**********************************************************/
      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

   

      /**********************************************************/
      /* One or more descriptors are readable.  Need to         */
      /* determine which ones they are.                         */
      /**********************************************************/
      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
	  {
         /*******************************************************/
         /* Check to see if this descriptor is ready            */
         /*******************************************************/
         if (FD_ISSET(i, &working_set))
         {
            
            /*-----------------A descriptor was found that was readable------------------*/
            desc_ready -= 1;

           
            /*------------Check to see if this is the listening socket-----------------*/
            
            if (i == ds)
            {
               printf("  Listening socket is readable\n");
              
               /*---------------- Accept connections--------------*/
               
               do
               {
                  new_sd = accept(ds, (struct sockaddr*) &aC,&lg);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }

                  /*--------------Add the new incoming connection to the master read set-------------*/
                  
                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;

                  /*--------------Loop back up and accept another incoming connection-----------------*/
                  
               } while(new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {

              if (FD_ISSET(i, &working_set)){
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;

               int id;

                rcv=recv(i, &id, sizeof(int), 0);
                 if ( rcv == -1){
                     perror("Serveur : pb de reception");
                        close_conn = TRUE;
                      }
                 if ( rcv == 0){
                     printf("Serveur :Socket est ferme");
                       close_conn = TRUE; 
                      }

                 rcv=recv(i, &aC, sizeof(struct sockaddr_in), 0);
                 if ( rcv == -1){
                     perror("Serveur : pb de reception");
                        close_conn = TRUE;
                      }
                 if ( rcv == 0){
                     printf("Serveur :Socket est ferme");
                       close_conn = TRUE; 
                      }

                  
                   sockets[id]=i;
                   memcpy(&sites[id], &aC, sizeof(struct sockaddr_in));
                 
                 
                 compteur++;

                 if(compteur==nbsites){

                  if(choix==0){
                    int n=1;
                    for(int i=2; i<=nbsites; i++){

                      memcpy(&SendToSites[0] , &sites[i-1] , sizeof(struct sockaddr_in));

                       snd=send(sockets[i] , &n, sizeof(int), 0);
                      if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                         }
                      if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                        }

                       snd=send(sockets[i] , SendToSites, sizeof(SendToSites), 0);
                       if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                          }
                        if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                         }
                         }

                       memcpy(&SendToSites[0] , &sites[nbsites] , sizeof(struct sockaddr_in));

                       snd=send(sockets[1] , &n, sizeof(int), 0);
                      if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                         }
                      if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                        }

                       snd=send(sockets[1] , SendToSites, sizeof(SendToSites), 0);
                       if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                          }
                        if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                         }

                 }

                 if(choix==1){

                  for(int i=1; i<=nbsites; i++){ 

                   snd=send(sockets[i] , &compteur, sizeof(int), 0);
                      if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                         }
                      if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                        }

                       snd=send(sockets[i] , sites, sizeof(sites), 0);
                       if ( snd == -1){
                           perror("Serveur : pb de send");
                           close_conn = TRUE;
                          }
                        if ( snd == 0){
                           printf("Serveur :Socket est ferme");
                           close_conn = TRUE;
                         }

                       printf("struct envoyé \nip= %s\nport= %d \n", inet_ntoa(sites[i].sin_addr),ntohs(sites[i].sin_port));


                 } 
                 } 


                       end_server = TRUE;


                 } 


               if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }
              }
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */

   } while (end_server == FALSE);
 
    for (int i=0; i <= max_sd; i++)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }


close(ds);



return 0;
}
