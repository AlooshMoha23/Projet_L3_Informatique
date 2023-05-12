struct Server
{
    char* addr;
    int port;
    int socket;

    struct sockaddr_in server_address;
};

int connectConnectionInterface(struct Server* serv){
    
    //CREATION DE LA SOCKET
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0){
        return -1;
    }

    //INITIALISATION DE LA sockaddr_in
    serv->server_address.sin_family = AF_INET;
    //server_address.sin_addr = inet_addr(serv->addr);
    inet_pton(AF_INET,serv->addr,&(serv->server_address.sin_addr)) ;
    serv->server_address.sin_port = htons(serv->port);
    socklen_t lgA = sizeof(struct sockaddr_in);

    //CONNEXION A L'INTERFACE
    int valConnect = connect(client_socket, (struct sockaddr *)&(serv->server_address), lgA);
    if(valConnect<0){
        return -1;
    }
    
    serv->socket = client_socket;
    return 1;
}

int sendId(struct Server* serv, int id){
    int sndWaitId = send(serv->socket, &id, sizeof(int), 0);
    if ( sndWaitId <= 0){
        return sndWaitId;
    }
    return 1;
}

int sendStatus(struct Server* serv, int status){
    int sndWait = send(serv->socket, &status, sizeof(status), 0);
    if ( sndWait <= 0){
        return sndWait;
    }
    return 1;
}

int sendStruct(struct Server* serv, struct sockaddr_in SendToMainServer){
    int sndWait = send(serv->socket, &SendToMainServer , sizeof(struct sockaddr_in), 0) ;
    if ( sndWait <= 0){
        return sndWait;
    }
    return 1;
}

int closeConnectionInterface(struct Server* serv){
    if(close(serv->socket)< 0){
        return -1;
    }
    
    return 0;
}
