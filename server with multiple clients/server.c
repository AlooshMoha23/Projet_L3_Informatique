#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
//https://www.tala-informatique.fr/wiki/index.php?title=C_pthread


// to compile: bash compile.script

GtkWidget *App;
GtkWidget *container1;
GtkWidget *processus;
GtkWidget *etat;
GtkWidget *etat2;
GtkWidget *etat3;

GtkBuilder *builder;// pointer unsed in connection with loading the xml file (interface.glade)

void *run_gtk_main(void *arg) {//first thread that would execute in pararell
    gtk_main();
    return NULL;
}





#define TRUE             1
#define FALSE            0

int main (int argc, char *argv[]){

	gtk_init(&argc, &argv); //for any gtk paramtres passed int the command ligne 

    builder=gtk_builder_new_from_file("interface.glade");

    App    = GTK_WIDGET(gtk_builder_get_object(builder, "App"));  //"APP" is the one from the interface

    g_signal_connect(App, "destroy", G_CALLBACK(gtk_main_quit), NULL); //when the window App is destroyed (when we leave the app)  call the callback gtk_main_quit
    gtk_builder_connect_signals(builder, NULL);

    container1 = GTK_WIDGET(gtk_builder_get_object(builder, "container1")); //connection with container1
    processus= GTK_WIDGET(gtk_builder_get_object(builder,"processus"));
    etat= GTK_WIDGET(gtk_builder_get_object(builder,"etat"));
    etat2= GTK_WIDGET(gtk_builder_get_object(builder,"etat2"));
    
  
    gtk_widget_show(App); //show the window

    pthread_t gtk_thread;//naming the thread1
    int thrd = pthread_create(&gtk_thread, NULL, run_gtk_main, NULL);
    if (thrd != 0) {
        perror("Error creating gtk_main thread");
        exit(1);
    }
    pthread_join(thrd, NULL);//jointing so it would work in parrel
    

   
/*******************socket***************************************************/

  if (argc != 2){
    printf("utilisation : %s port_serveur\n", argv[0]);
    exit(1);
    }




   int    i,k=0, len, rc, on = 1, socktrack[1024];
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   char   buffer[80];
   struct sockaddr_in  ServerInfo;
   struct sockaddr_in ClientInfo;
   fd_set        master_set;
   fd_set        working_set;
   

  socklen_t lg = sizeof(struct sockaddr_in);

   
   /*------------------------- Create socket to receive incoming connections on--------------------------*/
  
  
   listen_sd = socket(PF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }

   
   /*------------------------ Allow socket descriptor to be reuseable--------------------*/

   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }

  
   /*--------------------Set socket to be nonblocking.---------------*/
   
   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Bind the socket                                           */
   /*************************************************************/
  ServerInfo.sin_family = AF_INET ;
  ServerInfo.sin_addr.s_addr =INADDR_ANY;
  ServerInfo.sin_port = htons((short) atoi(argv[1])) ; 





   rc = bind(listen_sd, (struct sockaddr*)&ServerInfo, sizeof(ServerInfo));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }

   
   /*---------------Set the listen fonction--------------------*/
   
   rc = listen(listen_sd, 32);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }

  
   /*----------------Initialize the master fd_set---------------------*/
   
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);


   
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
            
            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
              
               /*---------------- Accept connections--------------*/
              
               
               do
               {
                
                  

                  new_sd = accept(listen_sd, (struct sockaddr*) &ClientInfo,&lg);
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
                  
               } while (new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               
               
                 
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }

                
                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
                     break;
                  }

                 

                 

                  char mes[500];

                  printf("Message recu par le clinet %d: %s \n",i, buffer) ;
                  sprintf(mes, "client (%d): %s",i,buffer);
                  
                  

                  if(k==0){
                        gtk_label_set_text(GTK_LABEL(etat),mes);
                        k=1;
                        }
                  else{
                      gtk_label_set_text(GTK_LABEL(etat2), mes);
                      k=0;
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
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */

   } while (end_server == FALSE);

   /*************************************************************/
   /* Clean up all of the sockets that are open                 */
   /*************************************************************/
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
}