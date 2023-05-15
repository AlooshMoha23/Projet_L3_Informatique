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
#define min(x,y) x<y?x:y;

#define MAX_CLIENTS 10

typedef struct {
    double x;
    double y;
    double radius;
    GdkRGBA color;
    const char* text;
    int socket_fd;
} Circle;

Circle circles[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t lock;
GtkWidget* drawing_area;  
gint width, height;


static void draw_circle(cairo_t* cr, double x, double y, double radius, const GdkRGBA* color,const char* text) {
    cairo_set_source_rgba(cr, color->red, color->green, color->blue, color->alpha);
    cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_move_to(cr, x-2, y-2);
    cairo_show_text(cr, text);
}

static gboolean on_draw(GtkWidget* widget, cairo_t* cr, gpointer data) {
    Circle* circle = (Circle*) data;

    draw_circle(cr, circle->x, circle->y, circle->radius, &circle->color,circle->text);

    return FALSE;
}


void* gui_thread(void* arg) {
    GtkWidget* window;
   
    GtkWidget* vbox;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Nodes");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 300, 300);
    gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 0);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), &circles[num_clients]);
    GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 0);

     gtk_widget_get_size_request(drawing_area, &width, &height);

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

    //Accept incoming connections and draw circles
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

            // Acquire lock to safely modify circles array
            pthread_mutex_lock(&lock);

            if (num_clients < MAX_CLIENTS) {
                // Create new circle and add to array
                  Circle new_circle;
                    new_circle.radius = 20;

                    // Center the circle horizontally
                     double cx = (width - 2 * new_circle.radius) / 2;
    
                     // Center the circle vertically
                     double cy = (height - 2 * new_circle.radius) / 2;


                    new_circle.x = (50 + num_clients * 100) +cx;
                    new_circle.y = 50 +cy;
                    new_circle.color.red = 0.0;
                    new_circle.color.green = 0.0;
                    new_circle.color.blue = 1.0;
                    new_circle.color.alpha = 1;
                    new_circle.text="ETAT";
                    new_circle.socket_fd=new_socket;
                    circles[num_clients++] = new_circle;
                    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));

                printf("New connection, socket fd is %d, num_clients is %d\n", new_socket, num_clients);
            }

            // Release lock
            pthread_mutex_unlock(&lock);
        }

        // Check if any of the client sockets have activity
         for (int i = 0; i < max_sd+1; i++) {
            sd = circles[i].socket_fd;
            if (FD_ISSET(sd, &use)) {
                        // Process message from client
                          int f;
                            while(1){
                            int rc = recv(new_socket, &f, sizeof(int), 0);
                                        if (rc < 0){
                                                perror("  recv() failed");
                                            
                                        }
                                        if (rc == 0)
                                        {
                                            printf("  Connection closed\n");
                                            
                                        }
                                        
                                        if(f==1){
                                            printf("%d\n",f);
                                            circles[i].color.green = 0.0;
                                            circles[i].color.red = 1.0;
                                            circles[i].color.blue = 0.0;
                                            circles[i].text="1";
                                            gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
                            
                                        }
                                        
                                        else{
                                            printf("%d\n",f);
                                            circles[i].color.green = 1.0;
                                            circles[i].color.red = 0.0;
                                            circles[i].color.blue = 0.0;
                                            circles[i].text="0";
                                            gtk_widget_queue_draw(GTK_WIDGET(drawing_area));

                                        }
                                            
                                    }
                    // Release lock
                    pthread_mutex_unlock(&lock);

                        }
                    }
                }
            
        
    
 return NULL;
}













int main() {
pthread_t gui_tid, server_tid;

// Initialize mutex
pthread_mutex_init(&lock, NULL);

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

// Destroy mutex
pthread_mutex_destroy(&lock);

return 0;
}