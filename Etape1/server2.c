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


#include "fonctions.h"
//https://www.tala-informatique.fr/wiki/index.php?title=C_pthread


// to compile: bash compile.script

GtkWidget *App;
GtkWidget *Container;

cairo_t *node;


GtkBuilder *builder;// pointer unsed in connection with loading the xml file (interface.glade)
int n;



int main (int argc, char *argv[]){

	gtk_init(&argc, &argv); //for any gtk paramtres passed int the command ligne 
   GTK(App, Container,builder);


 





/*******************thread***************************************************/
/*      thread to handle gtk main loop                                    */
/*****************************************************************************/
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



  
  


   struct sockaddr_in  ServerInfo;
   struct sockaddr_in ClientInfo;
  

 
  
  socklen_t lg = sizeof(struct sockaddr_in);

   
   /*------------------------- Create socket to receive incoming connections on--------------------------*/
  
    
   int listen_sd = socket(PF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(1);
   }

   printf("sockt created...\n");
  
   /*************************************************************/
   /* Bind the socket                                           */
   /*************************************************************/
  ServerInfo.sin_family = AF_INET ;
  ServerInfo.sin_addr.s_addr =INADDR_ANY;
  ServerInfo.sin_port = htons((short) atoi(argv[1])) ; 





   int bnd = bind(listen_sd, (struct sockaddr*)&ServerInfo, sizeof(ServerInfo));
   if (bnd < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(1);
   }
 printf("sockt bind...\n");
   
   /*---------------Set the listen fonction--------------------*/
   
   int rc = listen(listen_sd, 10);
   
   if (rc < 0)
   {
      perror("listen() failed");
      exit(1);
   }
    
  printf("server listenning...\n");
   /*----------------Initialize the master fd_set---------------------*/

int acp= accept(listen_sd, (struct sockaddr*) &ClientInfo,&lg);
                  if (acp < 0)
                  {
                     perror("  accept() failed");
                        
                     exit(0);
                  }


int width = gtk_widget_get_allocated_width(Container);
int height = gtk_widget_get_allocated_height(Container);

draw_node(node, width/2, height/2);


printf("connection accepted...\n");

while(1){


   int rcv=recv(acp, &n, sizeof(int), 0);

   if(rcv==-1){
      perror("  recv failed");
      exit(0);

   }

   if(rcv==0){
      perror(" server: client closed");
      exit(0);

   }

   printf("message recieved...\n");


  
  
}













return 0;
}










