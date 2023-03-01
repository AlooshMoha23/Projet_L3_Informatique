#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h> 
#include <sys/socket.h>
#include <netdb.h>
#include<arpa/inet.h>
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


int main(int argc, char *argv[]){

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
    

   
    /*******************socket*************************/
    if (argc != 2){
    printf("utilisation : %s port_serveur\n", argv[0]);
    exit(1);
    }

  
  int ds = socket(PF_INET, SOCK_STREAM, 0);


  if (ds == -1){
    perror("Serveur : pb creation socket :");
    exit(1); 
  }
  
 
  printf("Serveur : creation de la socket rÃ©ussie \n");
  gtk_label_set_text(GTK_LABEL(processus), "Serveur en ecoute:");
  


  struct sockaddr_in  ServerInfo ;
  ServerInfo.sin_family = AF_INET ;
  ServerInfo.sin_addr.s_addr =INADDR_ANY;
  ServerInfo.sin_port = htons((short) atoi(argv[1])) ; 

  int res=bind(ds, (struct sockaddr*)&ServerInfo, sizeof(ServerInfo));
   if (res == -1){
    perror("Serveur : pb creation socket :");
    exit(1);
   }
  
  int lsn=listen(ds, 7);
   if ( lsn == -1){
    perror("Serveur : pb d'ecoute:");
    exit(1); 
  }
  struct sockaddr_in ClientInfo;

  socklen_t lg = sizeof(struct sockaddr_in);
  int k=0;

  int dSC= accept(ds, (struct sockaddr*) &ClientInfo,&lg);
    if ( dSC == -1){
    perror("Serveur : pb de connexion au client:");
    exit(1); 
  }
  
  while(1){
    
  
    char ip[100];
    short unsigned int port;
   
   inet_ntop(AF_INET,(struct sockaddr*)&ClientInfo.sin_addr,ip,INET_ADDRSTRLEN);
   port=ntohs(ClientInfo.sin_port);

  char msg [4000] ;
  
  int rcv=recv(dSC, msg, sizeof(msg), 0);
    if ( rcv == -1){
    perror("Serveur : pb de reception");
    exit(1); 
  }
    if ( rcv == 0){
    printf("Serveur :Socket est ferme");
    exit(1); 
  }
  char mes[500];

  printf("Message recu par le clinet %d: %s \n",port, msg) ;
  sprintf(mes, "%s (%d): %s",ip,port,msg);
  if(k==0){
  gtk_label_set_text(GTK_LABEL(etat), mes);}
  else{
  gtk_label_set_text(GTK_LABEL(etat2), mes);
  }
  
 
  }

  int cls=close (dSC);
  if ( cls == -1){
    perror("Serveur : pb de fermeture de socket de client");
    exit(1); 
  }
    char chav[10];
  printf("suspension");
  scanf("%s",chav);
   int Cs=close (ds) ;
    if ( Cs == -1){
    perror("Serveur : pb de fermeture de socket");
    exit(1); 
  }
  
  printf("\nServeur : je termine\n");


  pthread_exit(&thrd);

return 0;
}

   



