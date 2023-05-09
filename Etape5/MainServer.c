#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sys/select.h>
#include<string.h>
#include <fcntl.h>
#include <time.h>
#define min(x,y) x<y?x:y;
//LD_PRELOAD=/lib/x86_64-linux-gnu/libpthread.so.0 ./p 3000 etat.txt to compile for me 

typedef struct {
    double x;
    double y;
    double radius;
    GdkRGBA color;
    const char* text;
    int socket_fd; 
    int * con_to;
    double angle;
    struct sockaddr_in addrs;
    char* nom_etat;
    int count_i;
    int indice;
    GdkRGBA last_color;
} Circle;

typedef struct {
    double x;
    double y;
    const char* text;
    GdkRGBA color;
} Text;

typedef struct {
    int numEtat;
    GdkRGBA color;
    char* descEtat;
} Etat;

char ** display_adrr;//affichage d'adresses
char ** indices;//affichage d'indice
Circle *circles = NULL;
int num_clients = 0;//compteur client
int count_etat=0;
Etat * etats=NULL;
GtkWidget* drawing_area;  
GtkWidget *state_list_box;

//parametre du graphe
double center_x = 500;
double center_y = 500 ;
double radius = 280 ;
double angle_step=0;

int MAX_CLIENTS=0;
int closed_count=0;
int count_start=0;
int kam=0;
int zoom_display=0;
Text label;

void on_submit(gpointer *data){
    const char *dataa = gtk_entry_get_text(GTK_ENTRY(data));
    if(strlen(dataa) == 0 || atoi(dataa)<=0){
        zoom_display=0;
        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    }
    else if( atoi(dataa)>num_clients){
        zoom_display= 0;

    }
    else{
        zoom_display=atoi(dataa);
        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
        printf("submited %s: %d\n",dataa,zoom_display);
    }
}

void update_status_list(int client_num, char* nom_etat,int row_index) {
     GtkListBox* list_box = GTK_LIST_BOX(state_list_box);

     char* label_text = (char *)calloc(566, sizeof(char));
    if (label_text == NULL) {
        fprintf(stderr, "Error: Memory allocation for label_text failed.\n");
        return;
    }

    // Create the label text
    time_t current_time = time(NULL);
    struct tm *tm_info = localtime(&current_time);
    char time_str[9]; 
    strftime(time_str, sizeof(time_str), "%T", tm_info); 
    sprintf(label_text, "%s     : Site %d (%s)", time_str, client_num, nom_etat);
    printf("LIST: %s\n", label_text);

    GtkWidget* label = gtk_label_new(label_text);

    GtkListBoxRow *row = gtk_list_box_get_row_at_index(list_box, row_index);
    if (row == NULL) {
        label = gtk_label_new(label_text);
        gtk_list_box_insert(list_box, label, row_index);
        gtk_widget_show(label);
    } else {
        label = gtk_bin_get_child(GTK_BIN(row));
        gtk_label_set_text(GTK_LABEL(label), label_text);
        gtk_widget_show(label);
    }
    //gtk_list_box_insert(GTK_LIST_BOX(list_box),label, -1);
    kam ++;
    printf("Row inserted %d.\n",kam);
   // gtk_widget_show(label);
    
    free(label_text);

}

//Botton pour basculer les affichage des adresses et indice de sites
static void on_check_button_toggled(GtkToggleButton *toggle_button, gpointer user_data)
{
    Circle *circles = (Circle *)user_data;
    gboolean active = gtk_toggle_button_get_active(toggle_button);
    for (int i = 0; i < num_clients; i++)
    {
        Circle *circle = &circles[i];
        int f=circle->indice-1;
        if (active)
        {
            circle->text = display_adrr[i];
        }
        else
        {
            circle->text = indices[f];
        }
    }
    gtk_widget_queue_draw(drawing_area);
}
//fonction pour dessiner les cercles
static void draw_circle(cairo_t* cr, double x, double y, double radius,double angle, const GdkRGBA* color,const char* text, char* texti) {
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_fill(cr);
    // Draw the border for circle
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_stroke(cr);
    
    cairo_set_font_size(cr, 7.5);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);


    if (angle >= M_PI_2 && angle <= 3*M_PI_2) {
        // Left half of ring
        cairo_text_extents_t extents;
        cairo_text_extents(cr,text, &extents);
        double x1=extents.width;
        cairo_save(cr); // save current state of the context
        cairo_translate(cr, x, y); // move origin to center of circle
        cairo_rotate(cr, angle); // rotate coordinate system around origin by angle
        cairo_scale(cr, -1, -1); // mirror text horizontally
        cairo_move_to(cr, -radius-x1-10, 0); // move text cursor to edge of circle
        cairo_show_text(cr, text); // show text
        cairo_restore(cr); // restore previous state of the context

        cairo_text_extents(cr,texti, &extents);
        cairo_save(cr); // save current state of the context
        cairo_set_source_rgba(cr, 1.0, 0.6, 0.0, 1.0);
        cairo_translate(cr, x, y); // move origin to center of circle
        cairo_rotate(cr, angle); // rotate coordinate system around origin by angle
        cairo_scale(cr, -1, -1); // mirror text horizontally
        cairo_move_to(cr, -radius-extents.width-x1-15, 0); // move text cursor to edge of circle
        cairo_show_text(cr, texti); // show text
        cairo_restore(cr); // restore previous state of the context
    } else {
        // Right half of ring
        cairo_text_extents_t extents;
        cairo_text_extents(cr,text, &extents);
        double x1=extents.width;
        cairo_save(cr); // save current state of the context
        cairo_translate(cr, x, y); // move origin to center of circle
        cairo_rotate(cr, angle); // rotate coordinate system around origin by angle
        cairo_move_to(cr, radius+10, 0); // move text cursor to edge of circle
        cairo_show_text(cr, text); // show text
        cairo_restore(cr); // restore previous state of the context

        cairo_text_extents(cr,texti, &extents);
        cairo_save(cr); // save current state of the context
        cairo_set_source_rgba(cr, 1.0, 0.6, 0.0, 1.0);
        cairo_translate(cr, x, y); // move origin to center of circle
        cairo_rotate(cr, angle); // rotate coordinate system around origin by angle
        cairo_move_to(cr, radius+x1+15, 0); // move text cursor to edge of circle
        cairo_show_text(cr, texti); // show text
        cairo_restore(cr); // restore previous state of the context
    }
}
//Fonction pour afficher du texte
void draw_text(GtkWidget *widget, cairo_t *cr, double x, double y, const char* text,const GdkRGBA* color)
{
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 14.0);
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_move_to(cr, x,y);
    cairo_show_text(cr, text);
    return FALSE;
}
//Fonction de dessin 
static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    //text labels
    Text label1;
    label1.text="Suivi de l'éxécution";
    label1.x=900;
    label1.y=880;
    label1.color.red=1.0;
    label1.color.blue=1.0;
    label1.color.green=1.0;
    label1.color.alpha=1.0;
    draw_text(widget , cr,label1.x, label1.y, label1.text,&label1.color);
    label.x=1069;
    label.y=880;
    draw_text(widget , cr,label.x, label.y, label.text,&label.color);
    
    if(zoom_display==0){
    
        // Draw a line between each two circles
        for (int i = 0; i < num_clients; i++) {
            for (int j = 0; j < num_clients; j++)
            {
                if(circles[i].con_to[j]==1){
                    Circle* circle1 = &circles[j];
                    Circle* circle2 = &circles[i];
                    cairo_set_line_width(cr, 2);
                    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
                    cairo_move_to(cr, circle1->x, circle1->y);
                    cairo_line_to(cr, circle2->x, circle2->y);
                    cairo_stroke(cr);

                }
            }
        }
        // Iterate over all circles and draw each one
        for (int i = 0; i < num_clients; i++) {
            Circle* circle = &circles[i];
            draw_circle(cr, circle->x, circle->y, circle->radius,circle->angle,&circle->color, circle->text,circle->nom_etat);
        }
    }else{
        for (int i = 0; i < num_clients; i++) {
            Circle* circle = &circles[i];

            if (circle->indice == zoom_display) {

                for (int j = 0; j < num_clients; j++) {
                    if (circle->con_to[j] == 1) {
                        Circle* connected_circle = &circles[j];
                        cairo_set_line_width(cr, 2);
                        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.5);
                        cairo_move_to(cr, connected_circle->x, connected_circle->y);
                        cairo_line_to(cr, circle->x, circle->y);
                        cairo_stroke(cr);
                        draw_circle(cr, circle->x, circle->y, circle->radius,circle->angle,&circle->color, circle->text,circle->nom_etat);
                        draw_circle(cr, connected_circle->x, connected_circle->y, connected_circle->radius,connected_circle->angle,&connected_circle->color, connected_circle->text,connected_circle->nom_etat);
                    }
                }
            }

        }
    }
    
    return FALSE;
}



//Thread interface
void* gui_thread(void* arg) {
    GtkWidget* window;
    GtkWidget* hboxx;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Interface");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 1000);

    GdkRGBA color = { 0.0, 0.0, 0.2, 1.0 };  // blue color (RGBA fs)
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &color);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    hboxx = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), hboxx);



    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1300, 900);
    gtk_box_pack_start(GTK_BOX(hboxx), drawing_area, FALSE, FALSE, 0);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &circles[0]);

    GtkWidget *vboxx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hboxx), vboxx, TRUE, FALSE, 0);


    GtkWidget *state_list_label = gtk_label_new("Suivre de prés:");
    gtk_widget_set_margin_top(state_list_label, 5);
    gtk_widget_set_margin_bottom(state_list_label, 5);
    gtk_box_pack_start(GTK_BOX(vboxx), state_list_label, FALSE, FALSE, 0);
    

    // Create a scrolled window to hold the state list
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 250, 700);
    gtk_box_pack_start(GTK_BOX(vboxx), scrolled_window, FALSE, FALSE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   
    
    // Create a list box to hold the state items
    state_list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), state_list_box);

            // Create the horizontal box container for the zoom buttons
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hboxx), vbox, TRUE, FALSE, 0);
    
    // Scroll to the bottom of the scrolled window
    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    if (adjustment != NULL) {
        double max_value = gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size(adjustment);
        gtk_adjustment_set_value(adjustment, max_value);
    }


    GtkWidget *check_button = gtk_check_button_new_with_label("Afficher les adresse ips");
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);
    g_signal_connect(check_button, "toggled", G_CALLBACK(on_check_button_toggled), &circles[0]);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(vbox), input_box, FALSE, FALSE, 0);

    GtkWidget *input_label = gtk_label_new("Zoom sur un site: ");
    gtk_box_pack_start(GTK_BOX(input_box), input_label, FALSE, FALSE, 0);



    GtkWidget *input_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(input_box), input_entry, FALSE, FALSE, 0);


    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(input_box), submit_button, FALSE, FALSE, 0);

   g_signal_connect_swapped(submit_button, "clicked", G_CALLBACK(on_submit), input_entry);



    gtk_widget_show_all(window);
    gtk_main();
    return NULL;
}
//Thread serveur avec select
void* server_thread(void* arg) {
    char **argv = arg;
    int server_fd, new_socket, max_sd, sd, activity;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    fd_set readfds;
    // Creation de la socket du serveur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // Plusieurs connexions sur le meme port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // Nommer la socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // Ecouter les connexions
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Serveur en ecoute\n");
    // Accepter les nouvelles conexion et dessiner les circles
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    max_sd = server_fd;
    fd_set use;
    while (1) {
        use=readfds;
        for (int i = 0 ; i < num_clients ; i++) {
            sd = circles[i].socket_fd;
            // If valid socket descriptor then add to read list
            if(sd > 0) {
                FD_SET(sd, &use);
            }
            // Highest file descriptor number, need it for the select function
            if(sd > max_sd) {
                max_sd = sd;
            }
        }
        // Wait for activity on any of the sockets
        activity = select( max_sd + 1 , &use , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error");
        }
        struct sockaddr_in adrr;
        // If incoming connection, accept and add new circle
        if (FD_ISSET(server_fd, &use)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&adrr, (socklen_t*)&addrlen))<0) {
                perror("accept");
                continue;
            }
           
            printf("Client:%s:%d\n",inet_ntoa(adrr.sin_addr),ntohs(adrr.sin_port));
            if (num_clients < MAX_CLIENTS) {
                // Create new circle and add to array
                    Circle new_circle;
                    new_circle.con_to=(int *) malloc(MAX_CLIENTS * sizeof(int));
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                         new_circle.con_to[i] = -1;
                    }   
                   if(MAX_CLIENTS<50){
                    new_circle.radius = 20;
                    }
                    else if(MAX_CLIENTS<100){
                         new_circle.radius = 15;
                         radius=400;
                    }
                    else if(MAX_CLIENTS<150){
                         new_circle.radius = 10;
                         radius=300;
                    }
                    else if(MAX_CLIENTS<300){
                        new_circle.radius=5;
                        radius=400;
                    }
                    new_circle.color.alpha = 1;
                    new_circle.socket_fd=new_socket;
                    new_circle.addrs=adrr;
                    new_circle.nom_etat="Connecté";
                    char strr[30];
                    sprintf(strr,"%s(%d)",inet_ntoa(adrr.sin_addr),ntohs(adrr.sin_port));
                    strcpy(display_adrr[num_clients],strr);  
                    new_circle.text=display_adrr[num_clients];
                    new_circle.indice=0;
                    new_circle.count_i=0;
                    circles[num_clients] = new_circle;
                    num_clients++;
                  }
                printf("Nouvelle connexion, socket fd  %d, numéro client  %d\n", new_socket, num_clients-1);
            }
           else{
        // Check if any of the client sockets have activity
                for (int i = 0; i < num_clients; i++) {
                sd = circles[i].socket_fd;
                if (FD_ISSET(sd, &use)) {
                    char buffer[sizeof(struct sockaddr_in) ]={0};
                                int n = recv(sd, buffer, sizeof(buffer), 0);
                                circles[i].count_i++;
                                    if (n == -1) {
                                        printf("erreur recv\n");
                                        continue;
                                    } else if (n == 0) {
                                        // connection closed by client
                                        printf("Socket %d fermée par le client\n",sd);
                                        circles[i].color = circles[i].last_color;
                                        circles[i].nom_etat="Déconnecté";
                                        update_status_list(circles[i].indice, circles[i].nom_etat,i);
                                        close(sd);
                                        FD_CLR(sd, &use); 
                                        circles[i].socket_fd = -1;
                                        closed_count++;
                                        if(closed_count==MAX_CLIENTS){
                                        label.text="(Exécution terminée)";
                                        label.color.red=1.0;
                                        label.color.blue=0.0;
                                        label.color.green=0.0;
                                        label.color.alpha=1.0;}
                                        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                                        
                                        
                                        
                                    } else {
                                            if(n == sizeof(struct sockaddr_in)) {
                                                struct sockaddr_in server_address;
                                                memcpy(&server_address, buffer, sizeof(struct sockaddr_in ));
                                                printf("Adresse récue %s: %d\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port) );
                                                for(int j=0;j<num_clients;j++){
                                                    printf("Adresse du client %d %s: %d\n",j,inet_ntoa(circles[j].addrs.sin_addr),ntohs(circles[j].addrs.sin_port) );
                                                    if (memcmp(&circles[j].addrs.sin_addr,&server_address.sin_addr, sizeof(server_address.sin_addr) )== 0 &&
                                                        memcmp(&circles[j].addrs.sin_port,&server_address.sin_port, sizeof(server_address.sin_port) )== 0 ) {
                                                        circles[i].con_to[j]=1;
                                                        printf("Comparaison reussite : %d\n",circles[i].con_to[j]);
                                                        }
                                                    }
                                                }
                                            else if (n == sizeof(int)) {
                                                    if(circles[i].count_i==1){
                                                        int f;
                                                        
                                                        memcpy(&f, buffer, sizeof(f));
                                                        int k=f-1;
                                                        printf("L'indice du site: %d\n", f);
                                                        circles[i].indice=f;
                                                        char str[10];
                                                        sprintf(str,"Site %d",f);
                                                        strcpy(indices[k], str);
                                                        circles[i].text=indices[k];
                                                        circles[i].x = center_x + radius * cos(k * angle_step);
                                                        circles[i].y = center_y + radius * sin(k * angle_step);
                                                        circles[i].angle=k*angle_step;
                                                        printf("Coordonnées x:%f y:%f\n",circles[i].x,circles[i].y);
                                                        count_start++;
                                                        label.text="(Connexion)";
                                                        label.color.red=1.0;
                                                        label.color.blue=0.4;
                                                        label.color.green=0.0;
                                                        label.color.alpha=1.0;
                                                        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                                                        update_status_list(circles[i].indice, circles[i].nom_etat,i);
                                                    }
                                                    else{
                                                        int f;
                                                        memcpy(&f, buffer, sizeof(f));
                                                        printf("Etat récu: %d\n", f);
                                                        for(int j=0;j<count_etat;j++){
                                            
                                                            if(f==etats[j].numEtat){

                                                                printf(" Socket %d:%d\n",sd,f);
                                                                circles[i].color=etats[j].color;
                                                                circles[i].last_color=etats[j].color;
                                                                circles[i].nom_etat=etats[j].descEtat;
                                                                label.text="(Exécusion en cours)";
                                                                label.color.red=0.0;
                                                                label.color.blue=0.0;
                                                                label.color.green=1.0;
                                                                label.color.alpha=1.0;
                                                                gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                                                                update_status_list(circles[i].indice, circles[i].nom_etat,i);
                                                            }                                                                        
                                                        }
                                                    }
                                            
                                            } else {
                                                    printf("Received invalid data:%s\n",buffer);
                                                    }
                                        }                  
                                        }
                    }
                    }                 
    } 
    for (int i=0; i <= num_clients; ++i)
   {
      if (FD_ISSET(i, &use))
         close(i);
   }
   return NULL;
}
int main(int argc, char *argv[]) {
    if (argc != 3){
          printf("utilisation : %s PORT fichier_config, \n", argv[0]);
          exit(1);
    }
    pthread_t gui_tid, server_tid;
    FILE* fp;
    char line[4000];
    int nbr_etats=0;
    int n =0;
    int choix=0;
    int fol=0;
    count_etat = 0;
    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    
    //get nombre d'etats
    while (fgets(line, sizeof(line), fp)) {
        fol++;
        if (line[0] == '#') {
              continue; // ignore lines that start with #
        }
        
        if(fol==2){
              sscanf(line, "%d", &n);
              printf("nombre client: %d\n", n);
        }
        if(fol==4){
             sscanf(line, "%d", &nbr_etats);
             printf("Nombre etat: %d\n", nbr_etats);
        }
        if(fol==8){
             choix=atoi(line);
             printf("Choix de Configuration : %d\n", choix);
             break;
        }
    
    }   
    //initialize a table
     etats = (Etat *) malloc(nbr_etats * sizeof(Etat));
    //intialize circles and adresses tables
    circles = (Circle *) malloc(n * sizeof(Circle));
    display_adrr = (char **)calloc(n, sizeof(char *));
    for (int i = 0; i < n; i++) {
        display_adrr[i] = (char *)calloc(50, sizeof(char));
    }
    indices = (char **)calloc(n, sizeof(char *));
    for (int i = 0; i < n; i++) {
        indices[i] = (char *)calloc(50, sizeof(char));
    }
    MAX_CLIENTS=n;
    angle_step=2*M_PI /n;
    int numE;
    char ** desc;
    desc = (char **)malloc(nbr_etats * sizeof(char *));
    for (int i = 0; i < nbr_etats; i++) {
         desc[i] = (char *)malloc(50 * sizeof(char));
    }
    if(choix==0){
         srand(time(NULL)); // seed the random number generator
         int i=0;
         while (fgets(line, sizeof(line), fp)) {
               if (line[0] == '#') {
                 continue; // ignore lines that start with #
                }
               char des[50];
               Etat newEtat;
               sscanf(line, "%d:%s", &numE,des);
               printf("Number: %d : %s\n", numE,des);
               strcpy(desc[count_etat],des);
               double r = (double)rand() / RAND_MAX;
               double red = (double)i / nbr_etats;
               double green = ((double)r + 0.001) / nbr_etats;
               double blue = 1.0 - ((double)i+1.0 / nbr_etats);
               newEtat.numEtat=numE;
               newEtat.descEtat=desc[count_etat];
               newEtat.color.red = red > 1.0 ? 1.0/(double)i : red;
               newEtat.color.green = green > 1.0 ? 1.0/(double)i+0.01 : green;
               newEtat.color.blue = blue < 0.0 ? 1.0/(double)i+0.02: blue;
               newEtat.color.alpha = 1;
                etats[count_etat++]=newEtat;
                i++;
            }
    }
    if(choix==1){
         while (fgets(line, sizeof(line), fp)) {
            if (line[0] == '#') {
                continue; // ignore lines that start with #
            }
            char des[50];
            Etat newEtat;
            char red[50];
            char green[50];
            char blue[50];
            sscanf(line, "%d:%s %s %s %s", &numE,des,red, green, blue);
            strcpy(desc[count_etat],des);
            newEtat.numEtat=numE;
            newEtat.descEtat=desc[count_etat];
            newEtat.color.red=atof(red);
            newEtat.color.green=atof(green);
            newEtat.color.blue=atof(blue);
            newEtat.color.alpha=1;
            etats[count_etat++]=newEtat;
        } 
    }
    for(int i=0; i<count_etat; i++){
        printf("%d %s %f %f %f\n", etats[i].numEtat,etats[i].descEtat, etats[i].color.red, etats[i].color.green, etats[i].color.blue);
    }   
    fclose(fp);
    // Create GUI thread
    if (pthread_create(&gui_tid, NULL, gui_thread, NULL)) {
        fprintf(stderr, "Error creating GUI thread\n");
        return 1;
    }
    // Create server thread
    if (pthread_create(&server_tid, NULL, server_thread, argv)) {
        fprintf(stderr, "Error creating server thread\n");
        return 1;
    }
    // Wait for threads to finish
    pthread_join(gui_tid, NULL);
    pthread_join(server_tid, NULL);
    free(etats);
    free(circles);
    free(display_adrr);
    free(indices);
    return 0;
}
