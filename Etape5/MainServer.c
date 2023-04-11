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
#define min(x,y) x<y?x:y;




typedef struct {
    double x;
    double y;
    double radius;
    GdkRGBA color;
    const char* text;
    int socket_fd; 
    int * con_to;
    struct sockaddr_in addrs;
    const char* nom_etat;
    
} Circle;
typedef struct {
    double x;
    double y;
    const char* text;
} Text;
typedef struct {
    int numEtat;
    GdkRGBA color;
    char* descEtat;
  
} Etat;

//struct sockaddr_in *add_c;
 char ** display_adrr;
 char ** indices;

Circle *circles = NULL;

int num_clients = 0;

int count_etat=0;

Etat * etats=NULL;

GtkWidget* drawing_area;  
double center_x = 550;
double center_y = 350 ;
double radius = 100;
double angle_step=0;

int MAX_CLIENTS=0;
double angle = 0;





static void on_check_button_toggled(GtkToggleButton *toggle_button, gpointer user_data)
{
    // Cast user_data to Circle array
    Circle *circles = (Circle *)user_data;

    // Get the new state of the check button
    gboolean active = gtk_toggle_button_get_active(toggle_button);

    // Change the text in each circle based on the state of the check button
    for (int i = 0; i < num_clients; i++)
    {
        Circle *circle = &circles[i];
        if (active)
        {
           
            circle->text = display_adrr[i];
        }
        else
        {
            circle->text = indices[i];
        }
    }

    // Redraw the circles
    gtk_widget_queue_draw(drawing_area);
}



static void draw_circle(cairo_t* cr, double x, double y, double radius, const GdkRGBA* color,const char* text, char* texti) {
    
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_fill(cr);


    // Draw the border for circle
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_stroke(cr);
    //circles texts
    cairo_set_source_rgba(cr, 255.0, 255.0, 255.0, 1.0);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr,x-30,y-35);
    cairo_show_text(cr, text);
    cairo_set_source_rgba(cr, 255.0, 255.0, 255.0, 1.0);
    cairo_set_font_size(cr, 12.0);
    cairo_move_to(cr,x+30,y);
    cairo_show_text(cr, texti);

}
void draw_text(GtkWidget *widget, cairo_t *cr, double x, double y, const char* text)
{

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12.0);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_move_to(cr, x,y);
    cairo_show_text(cr, text);

    return FALSE;
}

static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    //text labels
    Text label;
    label.text="Network ring Graph";
    label.x=450;
    label.y=750;
    draw_text(widget , cr,label.x, label.y, label.text);
   
    // Draw a line between each two circles
    for (int i = 0; i < num_clients; i++) {
        for (int j = 0; j < num_clients; j++)
        {
            if(circles[i].con_to[j]==1){
                Circle* circle1 = &circles[j];
                Circle* circle2 = &circles[i];
                cairo_set_line_width(cr, 2);
                cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
                cairo_move_to(cr, circle1->x, circle1->y);
                cairo_line_to(cr, circle2->x, circle2->y);
                cairo_stroke(cr);

            }
        }
        
  
        
    }
  

    // Iterate over all circles and draw each one
    for (int i = 0; i < num_clients; i++) {
        Circle* circle = &circles[i];
        draw_circle(cr, circle->x, circle->y, circle->radius, &circle->color, circle->text,circle->nom_etat);
    }
    

    return FALSE;
}




void* gui_thread(void* arg) {
    GtkWidget* window;
   
    GtkWidget* vbox;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Nodes");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 1000);
    GdkRGBA color = { 0.0, 0.0, 0.2, 1.0 };  // blue color (RGBA fs)
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &color);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    
    

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    


    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1000, 900);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 0);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &circles[0]);

    GtkWidget *check_button = gtk_check_button_new_with_label("Afficher les adresse ips");
    gtk_box_pack_start(GTK_BOX(vbox), check_button, FALSE, FALSE, 0);
    g_signal_connect(check_button, "toggled", G_CALLBACK(on_check_button_toggled), &circles[0]);
 

    gtk_widget_show_all(window);

    gtk_main();

    return NULL;
}

void* server_thread(void* arg) {
    int server_fd, new_socket, max_sd, sd, activity;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    

    fd_set readfds;

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow multiple connections to the same port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    // Bind the socket to a specific port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(3000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Serveur en ecoute\n");

    // Accept incoming connections and draw circles
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
                    new_circle.radius = 20;
                    new_circle.color.alpha = 1;
                    new_circle.socket_fd=new_socket;
                    new_circle.addrs=adrr;
                    //add_c[num_clients]=adrr;
                    char str[10];
                    sprintf(str,"Site %d",num_clients);
                    strcpy(indices[num_clients], str);
                    new_circle.text=indices[num_clients];
                    new_circle.nom_etat="Default";
                    char strr[30];
                    sprintf(strr,"%s(%d)",inet_ntoa(adrr.sin_addr),ntohs(adrr.sin_port));
                    strcpy(display_adrr[num_clients],strr);  
                    
                    
                    new_circle.x = center_x + radius * cos(angle);
                    new_circle.y = center_y + radius * sin(angle);
                    circles[num_clients] = new_circle;
                    num_clients++;
                    angle += angle_step;
                    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                  }
                printf("New connection, socket fd is %d, num_clients is %d\n", new_socket, num_clients-1);
              
            }
            

           else{
        // Check if any of the client sockets have activity
         for (int i = 0; i < num_clients; i++) {
            sd = circles[i].socket_fd;
            if (FD_ISSET(sd, &use)) {
                 char buffer[sizeof(struct sockaddr_in)] = {0};
                              
                                
                               int n = recv(sd, buffer, sizeof(buffer), 0);
                                if (n == -1) {
                                    printf("erreur recv\n");
                                    break;
                                   // usleep(1000); // sleep for 1 ms to avoid busy-waiting
                                } else if (n == 0) {
                                    // connection closed by client
                                    printf("Connection closed by client\n");
                                    break;
                                } else {
                                    if (n == sizeof(struct sockaddr_in)) {
                                        struct sockaddr_in server_address;
                                        memcpy(&server_address, buffer, sizeof(struct sockaddr_in ));
                                        printf("Received adresse %s: %d\n",inet_ntoa(server_address.sin_addr),ntohs(server_address.sin_port) );
                                          for(int j=0;j<num_clients;j++){
                                            printf("address of client %d %s: %d\n",j,inet_ntoa(circles[j].addrs.sin_addr),ntohs(circles[j].addrs.sin_port) );
                                          if (memcmp(&circles[j].addrs.sin_addr,&server_address.sin_addr, sizeof(server_address.sin_addr) )== 0 &&
                                          memcmp(&circles[j].addrs.sin_port,&server_address.sin_port, sizeof(server_address.sin_port) )== 0 ) {
                                          circles[i].con_to[j]=1;
                                          printf("See if compared : %d\n",circles[i].con_to[j]);
                                      }

                                    }

                                    } else if (n == sizeof(int)) {
                                        int f;
                                        memcpy(&f, buffer, sizeof(f));
                                        printf("Received integer: %d\n", f);
                                        for(int j=0;j<count_etat;j++){
                                        
                                        if(f==etats[j].numEtat){

                                            printf(" Socket %d:%d\n",sd,f);
                                            circles[i].color=etats[j].color;
                                            
                                            
                                            gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                            
                                        }
                                        
                                    
                                        }
                                    } else {
                                        printf("Received invalid data\n");
                                    }
                                
                            }
                
    
                
                                        
                                           
                                         
                                            
                                    }
                   
                        
                    }
                }
            
        
    }
 
    for (int i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &use))
         close(i);
   }
   return NULL;
}

int main() {

pthread_t gui_tid, server_tid;

 FILE* fp;
    char line[4000];
    int nbr_etats=0;

    fp = fopen("etat.txt", "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    //get nombre d'etats
    if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d", &nbr_etats);
        printf("Nombre etat: %d\n", nbr_etats);
    }
    
    //initialize a table 
    etats = (Etat *) malloc(nbr_etats * sizeof(Etat));
     count_etat = 0;
    int n =0;
    //get nbr clients
     if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d", &n);
        printf("nombre client: %d\n", n);
    }
    //intialize circles and adresses tables
    circles = (Circle *) malloc(n * sizeof(Circle));
    //add_c = ( struct sockaddr_in *) malloc(n * sizeof(struct sockaddr_in));
    display_adrr = (char **)malloc(n * sizeof(char *));
    for (int i = 0; i < n; i++) {
        display_adrr[i] = (char *)malloc(50 * sizeof(char));
    }
    indices = (char **)malloc(n * sizeof(char *));
    for (int i = 0; i < n; i++) {
        indices[i] = (char *)malloc(50 * sizeof(char));
    }
    MAX_CLIENTS=n;
    angle_step=2*M_PI /n;
    int choix;
 if (fgets(line, sizeof(line), fp)) {
        choix=atoi(line);

       printf("choice is : %d\n", choix);
    }


//choice, moi or vous

//if the clien choose letting us handle the colors, he should provid strings of all names of status
//example, he will provid "wait" and we will give a color for wait
int numE;

if(choix==0){
   

    srand(time(NULL)); // seed the random number generator
int i=0;
    char des[50];
     while (fgets(line, sizeof(line), fp)) {
        
       
       Etat newEtat;
       sscanf(line, "%d:%s", &numE,des);
       printf("Number: %d : %s\n", numE,des);
     
        strcpy(newEtat.descEtat,des);
      
          double r = (double)rand() / RAND_MAX;

    


        double red = (double)i / nbr_etats;
        double green = ((double)r + 0.001) / nbr_etats;
        double blue = 1.0 - ((double)i+1.0 / nbr_etats);
        newEtat.numEtat=numE;
       
        newEtat.color.red = red > 1.0 ? 1.0/(double)i : red;
        newEtat.color.green = green > 1.0 ? 1.0/(double)i+0.01 : green;
        newEtat.color.blue = blue < 0.0 ? 1.0/(double)i+0.02: blue;
        newEtat.color.alpha = 1;
        etats[count_etat++]=newEtat;
        i++;



    }
    


}

if(choix==1){
    
    char des[50];
     while (fgets(line, sizeof(line), fp)) {
        
       
        Etat newEtat;
        sscanf(line, "%d:%s", &numE,des);
       printf("Number: %d : %s\n", numE,des);
     
        strcpy(newEtat.descEtat,des);

        char red[50];
        char green[50];
        char blue[50];
        //Example wait 0.1 1.0 0.5
        fgets(line, 4000, fp);
        sscanf(line, "%d %s %s %s", &numE,red, green, blue);
        newEtat.numEtat=numE;
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
if (pthread_create(&server_tid, NULL, server_thread, NULL)) {
    fprintf(stderr, "Error creating server thread\n");
    return 1;
}
// Wait for threads to finish
pthread_join(gui_tid, NULL);
pthread_join(server_tid, NULL);

return 0;
}
