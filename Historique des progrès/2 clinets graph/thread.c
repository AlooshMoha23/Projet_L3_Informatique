#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#define MAX_PENDING 5
#define MAX_LINE_LENGTH 100

#define TRUE             1
#define FALSE            0
struct sockaddr_in send_adress;

void *server_thread(void *arg);
void *client_thread(void *arg);

int main(int argc, char **argv) {
    pthread_t server_tid;
    pthread_t client_tid;


    if(argc != 5){
        printf("entrer : ./thread port_serveur_local  (1:connexion server_local/0:distant_server)  adressse serveur distant port_serveur_extern\n");
        exit(0);
    }
    
    pthread_create(&server_tid, NULL, server_thread, argv);
    //sleep(3);
    pthread_create(&client_tid, NULL, client_thread, argv);

    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);

    return 0;
}

void *server_thread(void *param) {

    char **argv = param;
     int server_fd, client_fds[10], activity, max_fd, sd, i;
    int opt = 1;
    struct sockaddr_in address;
    struct sockaddr_in addressC;
    char buffer[1024] = {0};
     
    // create a socket for the server
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set socket options to reuse address and port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    // set the server address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));
    
    // bind the socket to the address and port
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    // set all client file descriptors to 0
    for (i = 0; i <10; i++) {
        client_fds[i] = 0;
    } 
    fd_set reads_fd, using_fd;
        FD_ZERO(&reads_fd);
        
        // add the server fd to the fd set
        FD_SET(server_fd, &reads_fd);
        max_fd = server_fd;

    
    while (1) {
         using_fd=reads_fd;
        // add all client fds to the fd set
        for (i = 0; i < 10; i++) {
            sd = client_fds[i];
            if (sd > 0) {
                FD_SET(sd, &using_fd);
            }
            if (sd > max_fd) {
                max_fd = sd;
            }
        }
        
        // wait for activity on any of the fds
        activity = select(max_fd + 1, &using_fd, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }
        
        // handle activity on the server fd
        if (FD_ISSET(server_fd, &using_fd)) {
            // accept a new connection
            

            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&addressC, (socklen_t*)&addressC)) < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }
            
            // set the new client socket to non-blocking
            int flags = fcntl(new_socket, F_GETFL, 0);
            if (flags < 0) {
                perror("fcntl failed");
                exit(EXIT_FAILURE);
            }
            if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("fcntl failed");
                exit(EXIT_FAILURE);
            }
                   for (i = 0; i < 10; i++) {
            if (client_fds[i] == 0) {
                client_fds[i] = new_socket;
                printf("New connection, socket fd is %d, IP is: %s, port is: %d\n", new_socket, inet_ntoa(addressC.sin_addr), ntohs(addressC.sin_port));
                break;
            }
        }
    }
    
    // handle activity on a client fd
    for (i = 0; i < 10; i++) {
        sd = client_fds[i];
        if (FD_ISSET(sd, &using_fd)) {
            // receive data from the client
            int rcv=recv(sd, buffer, sizeof(buffer)+1,0);
            if(rcv == 0) {
                // client disconnected
                getpeername(sd, (struct sockaddr*)&addressC, (socklen_t*)&addressC);
                printf("Host disconnected, socket fd is %d, IP is: %s, port is: %d\n", sd, inet_ntoa(addressC.sin_addr), ntohs(addressC.sin_port));
                close(sd);
                client_fds[i] = 0;
            }if (rcv==-1)
            {
                perror("Serveur : recv failed");
            }
            
                  char ip[100];
                  short unsigned int port;
              
                  inet_ntop(AF_INET,(struct sockaddr*)&addressC.sin_addr,ip,INET_ADDRSTRLEN);
                  port=ntohs(addressC.sin_port);
                  printf("Message recu par le clinet socket %d, %s (%d) : %s \n",sd,ip,port, buffer) ;
                char confirm[20]="confirm";
                  int rc=send(sd, confirm, sizeof(confirm)+1,0);
                    if (rc <= 0) {
                        perror("erreur lors de l'envoi du message au client\n");
                        exit(1);
                    }
                    else{
                        printf("Serveur: a repondu au client! \n");
                    }
            
        }
    }
}

return 0;
}

void *client_thread(void *param) {
    
    char **argv = param;
    int client_socket;
    struct sockaddr_in server_address,local_adress;



    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur lors de la creation de la socket client!\n");
        exit(1);
    }
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET,argv[3],&(server_address.sin_addr));
    server_address.sin_port = htons(atoi(argv[4]));
    send_adress=server_address;

    memset(&local_adress, 0, sizeof(local_adress));
    local_adress.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&(local_adress.sin_addr));
    local_adress.sin_port = htons(atoi(argv[1]));
    


        if(atoi(argv[2])==1){
            send_adress=local_adress;
        if (connect(client_socket, (struct sockaddr *) &local_adress, sizeof(local_adress)) < 0 ) {
            printf("demande de connexion echoué\n");
        }else{
            printf("Demande de connexion reussie\n");
        }
    

        char m[20]="active";
        char recM[20];
        char serM[1000];
        int snd=send(client_socket, m, strlen(m)+1, 0) ;
        if ( snd <= 0){
            perror("Client : pb d'envoi de message :\n");
            exit(1); 
        }
        printf("Client : message envoyé au serveur!\n");
        if(recv(client_socket, recM, sizeof(recM)+1, 0)>0){
            printf("client: reponse du serveur : %s \n",recM);
                int sock1 = socket(AF_INET, SOCK_STREAM, 0);
                if (sock1 == -1) {
                    perror("socket");
                    exit(1);
                }//
             struct sockaddr_in remote_server_address;
             memset(&remote_server_address, 0, sizeof(remote_server_address));
             remote_server_address.sin_family = AF_INET;
             inet_pton(AF_INET,"127.0.0.1",&(remote_server_address.sin_addr));
             remote_server_address.sin_port = htons(atoi("3000"));
        
              if (connect(sock1, (struct sockaddr *) &remote_server_address, sizeof(remote_server_address)) < 0 ) {
            printf("demande de connexion  a l'interface echoué\n");
            }else{
                printf("Client:Demande de connexion  a l'interface reussie\n");
            }
            int snd=send(sock1,(struct sockaddr *) &send_adress, sizeof(send_adress), 0);
            if ( snd <= 0){
                perror("Client : pb d'envoi de message a l'interface:\n");
                exit(1); 
            }else{
             printf("Client : message envoyé a l'interface!\n");
            }
            int rc=recv(sock1,serM,strlen(serM)+1,0);
             if ( rc <= 0){
                perror("Client : pb de reception de message envoyé par le serveur:\n");
                exit(1); 
            }else{
             printf("Client : message envoyé par l'interface : %s\n",serM);
            }


            



        }
        
        close(client_socket);
        }else{
            send_adress=server_address;
            if (connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) < 0 ) {
            printf("demande de connexion echoué\n");
        }else{
            printf("Demande de connexion reussie\n");
        }
    

        char m[20]="active";
        char recM[20];
        char serM[1000];
        int snd=send(client_socket, m, strlen(m)+1, 0) ;
        if ( snd <= 0){
            perror("Client : pb d'envoi de message :\n");
            exit(1); 
        }
        printf("Client : message envoyé au serveur!\n");
        if(recv(client_socket, recM, sizeof(recM)+1, 0)>0){
            printf("client: reponse du serveur : %s \n",recM);
                int sock1 = socket(AF_INET, SOCK_STREAM, 0);
                if (sock1 == -1) {
                    perror("socket");
                    exit(1);
                }//
             struct sockaddr_in remote_server_address;
             memset(&remote_server_address, 0, sizeof(remote_server_address));
             remote_server_address.sin_family = AF_INET;
             inet_pton(AF_INET,"127.0.0.1",&(remote_server_address.sin_addr));
             remote_server_address.sin_port = htons(atoi("3000"));
        
              if (connect(sock1, (struct sockaddr *) &remote_server_address, sizeof(remote_server_address)) < 0 ) {
            printf("demande de connexion  a l'interface echoué\n");
            }else{
                printf("Client:Demande de connexion  a l'interface reussie\n");
            }
            int snd=send(sock1,(struct sockaddr *) &send_adress, sizeof(send_adress), 0);
            if ( snd <= 0){
                perror("Client : pb d'envoi de message a l'interface:\n");
                exit(1); 
            }else{
             printf("Client : message envoyé a l'interface!\n");
            }
            int rc=recv(sock1,serM,strlen(serM)+1,0);
             if ( rc <= 0){
                perror("Client : pb de reception de message envoyé par le serveur:\n");
                exit(1); 
            }else{
             printf("Client : message envoyé par l'interface : %s\n",serM);
            }


            



        }
        
        close(client_socket);
            
            

        }
    return NULL;
}

