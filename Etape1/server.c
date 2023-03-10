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

GtkWidget *window;
GtkWidget *drawing_area;
cairo_t *node;

int n;




int main (int argc, char *argv[]){

	gtk_init(&argc, &argv); //for any gtk paramtres passed int the command ligne 
  // GTKInit(window, drawing_area);


   window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "INTERFACE");
   gtk_window_set_default_size(GTK_WINDOW(window), 800, 900);
   drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);

   gtk_widget_show_all(window);
   node = gdk_cairo_create(gtk_widget_get_window(drawing_area));

   //g_signal_connect(G_OBJECT(drawing_area), "button-press-event", G_CALLBACK(on_drawing_area_button_press), NULL);

   

   
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



 draw_node(node, 600, 500);
  

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

   printf("message recieved: %d...\n",n);
  
  // update_nodeColor(node, int n);
   

  
}













return 0;
}










