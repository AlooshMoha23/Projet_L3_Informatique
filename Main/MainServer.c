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
GtkWidget *suivi;
double zoom_factor = 1.0;
//parametre du graphe
double center_x = 1750;
double center_y = 1750 ;
double radius =500;
double angle_step=0;

int MAX_CLIENTS=0;
int closed_count=0;
int count_start=0;
int kam=0;
int focus_display=0;
Text label;

void update_suivi(int i){
    char* label_text = (char *)calloc(566, sizeof(char));
    if (label_text == NULL) {
        fprintf(stderr, "Error: Memory allocation for label_text failed.\n");
        return;
    }
    if(i==0){
       label_text="Suivi de l'éxécution: Connexion";
    }else if(i==1){
        label_text="Suivi de l'éxécution: Exécution en cours";
    }else{
        label_text="Suivi de l'éxécution: Exécution terminée";
    }
    gtk_label_set_text(GTK_LABEL(suivi), label_text);
}

//Fonction pour afficher un site depuis son numéro
void on_submit(gpointer *data){
    const char *dataa = gtk_entry_get_text(GTK_ENTRY(data));
    if(strlen(dataa) == 0 || atoi(dataa)<=0){
        focus_display=0;
        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
    }
    else if( atoi(dataa)>num_clients){
        focus_display= 0;

    }
    else{
        focus_display=atoi(dataa);
        gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
        printf("submited %s: %d\n",dataa,focus_display);
    }
}

//Mise a jour de la liste défilante 
void update_status_list(int client_num, char* nom_etat,int row_index) {
    GtkListBox* list_box = GTK_LIST_BOX(state_list_box);
    char* label_text = (char *)calloc(566, sizeof(char));
    if (label_text == NULL) {
        fprintf(stderr, "Error: Memory allocation for label_text failed.\n");
        return;
    }

    time_t current_time = time(NULL);
    struct tm *tm_info = localtime(&current_time);
    char time_str[9]; 
    strftime(time_str, sizeof(time_str), "%T", tm_info); 

    sprintf(label_text, "%s     : Site %d (%s)", time_str, client_num, nom_etat);
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

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_stroke(cr);
    
    cairo_set_font_size(cr, 10);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    if (angle >= M_PI_2 && angle <= 3*M_PI_2) {// Coté gauche du graphe
        cairo_text_extents_t extents;
        cairo_text_extents(cr,text, &extents);
        double x1=extents.width;
        cairo_save(cr); 
        cairo_translate(cr, x, y); 
        cairo_rotate(cr, angle);
        cairo_scale(cr, -1, -1); 
        cairo_move_to(cr, -radius-x1-10, 0); 
        cairo_show_text(cr, text);
        cairo_restore(cr); 

        cairo_text_extents(cr,texti, &extents);
        cairo_save(cr); 
        cairo_set_source_rgba(cr, 1.0, 0.6, 0.0, 1.0);
        cairo_translate(cr, x, y); 
        cairo_rotate(cr, angle);
        cairo_scale(cr, -1, -1);
        cairo_move_to(cr, -radius-extents.width-x1-15, 0); 
        cairo_show_text(cr, texti);
        cairo_restore(cr); 
    } else { //coté droit
        cairo_text_extents_t extents;
        cairo_text_extents(cr,text, &extents);
        double x1=extents.width;
        cairo_save(cr);
        cairo_translate(cr, x, y); 
        cairo_rotate(cr, angle);
        cairo_move_to(cr, radius+10, 0);
        cairo_show_text(cr, text);
        cairo_restore(cr);

        cairo_text_extents(cr,texti, &extents);
        cairo_save(cr);
        cairo_set_source_rgba(cr, 1.0, 0.6, 0.0, 1.0);
        cairo_translate(cr, x, y);
        cairo_rotate(cr, angle);
        cairo_move_to(cr, radius+x1+15, 0);
        cairo_show_text(cr, texti);
        cairo_restore(cr); 
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

    double zoom = zoom_factor;
    cairo_scale(cr, zoom, zoom);
    if(focus_display==0){
    
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

            if (circle->indice == focus_display) {

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
                    else{
                         draw_circle(cr, circle->x, circle->y, circle->radius,circle->angle,&circle->color, circle->text,circle->nom_etat);

                    }
                }
            }

        }
    }
      
    return FALSE;
}
void update_drawing_area_size() {
    int new_width = (int)(3500 * zoom_factor);
    int new_height = (int)(3500 * zoom_factor);
    gtk_widget_set_size_request(drawing_area, new_width, new_height);
}

void on_zoom_in_button_clicked(GtkWidget *widget, gpointer data) {
    zoom_factor += 0.1;
    update_drawing_area_size();
    gtk_widget_queue_draw(drawing_area);
}

void on_zoom_out_button_clicked(GtkWidget *widget, gpointer data) {
    zoom_factor -= 0.1;
    update_drawing_area_size();
    gtk_widget_queue_draw(drawing_area);
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
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &circles[0]);

    GtkWidget *scrolled_window_drawing_area = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window_drawing_area, 1300, 1000);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window_drawing_area),GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    gtk_box_pack_start(GTK_BOX(hboxx), scrolled_window_drawing_area, FALSE, FALSE, 0);

    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window_drawing_area), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &circles[0]);


    GtkWidget *vboxx = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hboxx), vboxx, TRUE, FALSE, 0);

    GtkWidget *state_list_label = gtk_label_new("Suivre de prés:");
    gtk_widget_set_margin_top(state_list_label, 5);
    gtk_widget_set_margin_bottom(state_list_label, 5);
    gtk_box_pack_start(GTK_BOX(vboxx), state_list_label, FALSE, FALSE, 0);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, 250, 850);
    gtk_box_pack_start(GTK_BOX(vboxx), scrolled_window, FALSE, FALSE, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
   
    state_list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), state_list_box);

    GtkWidget *check_button = gtk_check_button_new_with_label("Afficher les adresse ips");
    gtk_box_pack_start(GTK_BOX(vboxx), check_button, FALSE, FALSE, 0);
    g_signal_connect(check_button, "toggled", G_CALLBACK(on_check_button_toggled), &circles[0]);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(hboxx), vbox, FALSE, FALSE, 0);
        
    GtkWidget *insts = gtk_label_new("-> Numéro de site:affichage site et ses connexions");
    gtk_widget_set_margin_top(insts, 10);
    gtk_box_pack_start(GTK_BOX(vbox), insts, FALSE, FALSE, 0);

    GtkWidget *instss = gtk_label_new("-> '_': Retour au graphe initial");
    gtk_widget_set_margin_top(instss, 10);
    gtk_box_pack_start(GTK_BOX(vbox), instss, FALSE, FALSE, 0);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top(input_box, 30);
    gtk_box_pack_start(GTK_BOX(vbox), input_box, FALSE, FALSE, 0);

    GtkWidget *input_label = gtk_label_new("Zoom sur un site: ");
    gtk_box_pack_start(GTK_BOX(input_box), input_label, FALSE, FALSE, 0);

    GtkWidget *input_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(input_box), input_entry, FALSE, FALSE, 0);

    GtkWidget *submit_button = gtk_button_new_with_label("Submit");
    gtk_box_pack_start(GTK_BOX(input_box), submit_button, FALSE, FALSE, 0);
    g_signal_connect_swapped(submit_button, "clicked", G_CALLBACK(on_submit), input_entry);

    GtkWidget * zoom_in_button = gtk_button_new_with_label("Zoom In");
    gtk_box_pack_start(GTK_BOX(vbox), zoom_in_button, FALSE, FALSE, 0);
    g_signal_connect(zoom_in_button, "clicked", G_CALLBACK(on_zoom_in_button_clicked), NULL);
    GtkWidget *zoom_out_button = gtk_button_new_with_label("Zoom Out");
    gtk_box_pack_start(GTK_BOX(vbox), zoom_out_button, FALSE, FALSE, 0);
    g_signal_connect(zoom_out_button, "clicked", G_CALLBACK(on_zoom_out_button_clicked), NULL);

    suivi = gtk_label_new("Suivi d'éxécution: ");
    gtk_widget_set_margin_top(suivi, 10);
    gtk_box_pack_start(GTK_BOX(vbox), suivi, FALSE, FALSE, 0);

    update_drawing_area_size();
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
    printf("Serveur en écoute\n");
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
                    new_circle.radius=20;
                    new_circle.con_to=(int *) malloc(MAX_CLIENTS * sizeof(int));
                    for (int i = 0; i < MAX_CLIENTS; i++) {
                         new_circle.con_to[i] = -1;
                    }   
                   if(MAX_CLIENTS<50){
                    radius=600;
                    }
                    else if(MAX_CLIENTS<100){
                         radius=800;
                    }
                    else if(MAX_CLIENTS<150){
                         radius=1000;
                    }
                    else if(MAX_CLIENTS<300){
                        new_circle.radius=15;
                        radius=1500;
                    }else{
                        new_circle.radius=10;
                        radius=1600;
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
                                        update_suivi(5);
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
                                                        count_start++;
                                                        update_suivi(0);
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
                                                                update_suivi(1);
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
