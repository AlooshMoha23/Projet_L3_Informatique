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
} Circle;
typedef struct {
    double x;
    double y;
    const char* text;
} Text;
typedef struct {
    int numEtat;
    GdkRGBA color;
    const char* descEtat;
  
} Etat;

struct sockaddr_in *add_c=NULL;
 char display_adrr[50][40];
 char indices[50][30];

Circle *circles = NULL;

int num_clients = 0;
int nbr_etats;
int count_etat=0;

Etat * etats=NULL;

GtkWidget* drawing_area;  
double center_x = 300;
double center_y = 200;
double radius = 100;
double angle_step;
int MAX_CLIENTS=0;


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



static void draw_circle(cairo_t* cr, double x, double y, double radius, const GdkRGBA* color,const char* text) {
    
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
    label.x=165;
    label.y=350;
    draw_text(widget , cr,label.x, label.y, label.text);
    Text guide;
    guide.text=" Guide couleurs:";
    guide.x=470;
    guide.y=70;
    draw_text(widget , cr,guide.x, guide.y, guide.text);
     Text map1;
    map1.text=" Envoi de messages";
    map1.x=500;
    map1.y=100;
    cairo_text_extents_t extents;
    cairo_text_extents(cr, map1.text, &extents);
    //green rectangular
    double text_width = extents.width;
    double text_height = extents.height;
    cairo_set_source_rgba(cr, 0.14902, 0.30196, 0.0, 1.0);
    cairo_rectangle(cr, map1.x -25, map1.y - text_height -8, text_width + 65, text_height + 50);
    cairo_fill(cr);
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_stroke(cr);

    draw_text(widget , cr,map1.x, map1.y, map1.text);
    Text map2;
    map2.text=" Récéption de messages";
    map2.x=500;
    map2.y=130;
    draw_text(widget , cr,map2.x, map2.y, map2.text);
    //guide circles
    Circle g1;
    g1.radius=10;
    g1.x=489;
    g1.y=95;
    g1.color.green = 0.50196;
    g1.color.red = 0.0;
    g1.color.blue = 0.50196;
    g1.color.alpha=1.0;
    draw_circle(cr, g1.x, g1.y, g1.radius, &g1.color, NULL);
    
    Circle g2;
    g2.radius=10;
    g2.x=489;
    g2.y=125;
    g2.color.green = 0.6;
    g2.color.red = 0.90196;
    g2.color.blue = 0.0;
    g2.color.alpha=1.0;
    draw_circle(cr, g2.x, g2.y, g2.radius, &g2.color, NULL);

   
   







    // Draw a line between each two circles
    for (int i = 0; i < num_clients-1; i++) {
        for (int j = 0; j < num_clients; j++)
        {
            if(circles[i].con_to[j]==1){
                Circle* circle1 = &circles[i];
                Circle* circle2 = &circles[j];
                cairo_set_line_width(cr, 2);
                cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.8);
                cairo_move_to(cr, circle1->x, circle1->y);
                cairo_line_to(cr, circle2->x, circle2->y);
                cairo_stroke(cr);

            }
        }
        
  
        
    }
  

    // Iterate over all circles and draw each one
    for (int i = 0; i < num_clients; i++) {
        Circle* circle = &circles[i];
        draw_circle(cr, circle->x, circle->y, circle->radius, &circle->color, circle->text);
    }
    

    return FALSE;
}




void* gui_thread(void* arg) {
    GtkWidget* window;
   
    GtkWidget* vbox;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Nodes");
    gtk_window_set_default_size(GTK_WINDOW(window), 700, 600);
    GdkRGBA color = { 0.0, 0.0, 0.2, 1.0 };  // blue color (RGBA values)
    gtk_widget_override_background_color(window, GTK_STATE_FLAG_NORMAL, &color);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    
    

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    


    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 600, 500);
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

        // If incoming connection, accept and add new circle
        if (FD_ISSET(server_fd, &use)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                continue;
            }

            

            if (num_clients < MAX_CLIENTS) {
                
                
                // Create new circle and add to array
                  Circle new_circle;
                
                  
                    
                    new_circle.radius = 30;
                    new_circle.color.red = 0.0;
                    new_circle.color.green = 0.0;
                    new_circle.color.blue = 1.0;
                    new_circle.color.alpha = 1;
                    new_circle.socket_fd=new_socket;
                    add_c[num_clients]=address;
                    printf("Port %d\n",ntohs(add_c[num_clients].sin_port));
                    char str[10];
                    sprintf(str,"Site %d",num_clients);
                    strcpy(indices[num_clients], str);
                    new_circle.text=indices[num_clients];
                    char strr[30];
                    sprintf(strr,"%s(%d)",inet_ntoa(address.sin_addr),ntohs(address.sin_port));
                    strcpy(display_adrr[num_clients],strr);  
                    new_circle.x = center_x + radius * cos(num_clients * angle_step);
                    new_circle.y = center_y + radius * sin(num_clients * angle_step);
                    circles[num_clients++] = new_circle;
                    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                  }
                printf("New connection, socket fd is %d, num_clients is %d\n", new_socket, num_clients-1);
               
            }
            

           else{
        // Check if any of the client sockets have activity
         for (int i = 0; i < max_sd+1; i++) {
            sd = circles[i].socket_fd;
            if (FD_ISSET(sd, &use)) {
                
                       

                          struct sockaddr_in server_address;
                          if(recv(sd, &server_address, sizeof(server_address), 0)>0){
                            for(int j=0;j<MAX_CLIENTS;j++){
                               if (memcmp(&add_c[j],&server_address, sizeof(server_address)) == 0) {
                                        circles[i].con_to[j]=1;
                                }

                            }
                          }
                          else{
                            continue;
                          }

                          int f;
                          
                            int rc = recv(sd, &f, sizeof(int), 0);
                                        if (rc < 0){
                                                perror("  recv() failed");
                                            
                                        }
                                        if (rc == 0)
                                        {
                                            printf("  Connection closed\n");
                                            continue;
                                            
                                        }
                                           

                                            
                                            
                                        for(int i=0;i<count_etat;i++){
                                        
                                        if(f==etats[i].numEtat){

                                            printf(" Socket %d:%d\n",sd,f);
                                            circles[i].color=etats[i].color;
                                            
                                            gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                            
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
    int nbr_etats;

    fp = fopen("etat.txt", "r");
    if (fp == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    //get nombre d'etats
    if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d", &nbr_etats);
        printf("First number: %d\n", nbr_etats);
    }
    //initialize a table 
    etats = (Etat *) malloc(nbr_etats * sizeof(Etat));
    int n =0;
    //get nbr clients
     if (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d", &n);
        printf("nombre client: %d\n", n);
    }
    //intialize circles and adresses tables
    circles = (Circle *) malloc(n * sizeof(Circle));
    add_c = ( struct sockaddr_in *) malloc(n * sizeof(struct sockaddr_in));
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
    for(int i=0; i<nbr_etats; i++){
        fgets(line, 4000, fp);

        sscanf(line, "%d", &numE);

         
          Etat newEtat;
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
        
    }



}

if(choix==1){

    for(int i=0; i<nbr_etats; i++){
        Etat newEtat;

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





 for(int i=0; i<nbr_etats; i++){
    
        
        printf("%d %f %f %f\n", etats[i].numEtat, etats[i].color.red, etats[i].color.green, etats[i].color.blue);
  
    }   













/**********************************************************************************************/
/* end read                                                                                    */
/**********************************************************************************************/















    
   

  
    

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
