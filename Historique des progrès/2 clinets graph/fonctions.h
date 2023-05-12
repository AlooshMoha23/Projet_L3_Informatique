#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>



void *GTK(GtkWidget *App,GtkBuilder *builder){
   

    builder=gtk_builder_new_from_file("interface.glade");
    App    = GTK_WIDGET(gtk_builder_get_object(builder, "App"));  //"APP" is the one from the interface
    g_signal_connect(App, "destroy", G_CALLBACK(gtk_main_quit), NULL); //when the window App is destroyed (when we leave the app)  call the callback gtk_main_quit
    gtk_builder_connect_signals(builder, NULL);
  
    gtk_widget_show(App); //show the window
}

void *drawingErea(GtkWidget *App,GtkWidget *drawing_area){
    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 800, 600);
    gtk_container_add(GTK_CONTAINER(App), drawing_area);
    
}

void *run_gtk_main(void *arg) {//first thread that would execute in pararell
    gtk_main();
    return NULL;
}




/*******************t***************************************************/
/*drawing                                    */
/*****************************************************************************/

static void draw_node(cairo_t *cr, int x, int y)
{
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, x, y, 10, 0, 2 * G_PI);//NODE_SIZE=10
    cairo_fill(cr);
}

static void draw_line(cairo_t *cr, int x1, int y1, int x2, int y2)
{
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);//whait
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int n = GPOINTER_TO_INT(data);  //convert from gpointer to int
    int i, j, k;
    int x[n], y[n];
    srand(time(NULL));

    for (i = 0; i < n; i++) {
        x[i] = (i + 1) * (800 / (n + 1));  //800=WIDTH
        y[i] = (rand() % (600 - 50)) + 50 / 2; //50 = NODE_SPACING   600=HEIGHT
        draw_node(cr, x[i], y[i]);
    }

    for (i = 0; i < n; i++) {
        j = (i + 1) % n;
        k = (i + 2) % n;
        draw_line(cr, x[i], y[i], x[j], y[j]);
        draw_line(cr, x[i], y[i], x[k], y[k]);
    }

    return FALSE;
}


int CheckingServerExists(struct sockaddr_in *ClientStruct, struct sockaddr_in  ServerInfo){ //* pointer for first element of the array
    char ipS[100];
    short unsigned int portS;
    portS=ntohs(ServerInfo.sin_port);
    inet_ntop(AF_INET,(struct sockaddr*)&ServerInfo.sin_addr,ipS,INET_ADDRSTRLEN);

    for(int i=0; i<sizeof(ClientStruct); i++){
        char ipC[100];
        short unsigned int portC;
        inet_ntop(AF_INET,(struct sockaddr*)&ClientStruct[i].sin_addr,ipC,INET_ADDRSTRLEN);
        portS=ntohs(ClientStruct[i].sin_port);
        
        if(ipS==ipC && portC==portS){
            return i;
        }
    }

    return 0;
}




int sendTCP(int sock, void* msg, int sizeMsg) {
        
     int snd = send(sock, msg, sizeMsg, 0);

     if (snd == -1)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("snd() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }
                  if (snd == 0)
                  {
                     printf("Connection closed\n");
                     close_conn = TRUE;
                     break;
                  } 
    return snd;
}

int recvTCP(int sock, void* msg, int sizeMsg) {
        
     int rcv = recv(sock, msg, sizeMsg, 0);

     if (rcv == -1)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("rcv() failed");
                        close_conn = TRUE;
                     }
                     break;
                  }
                  if (rcv == 0)
                  {
                     printf("Connection closed\n");
                     close_conn = TRUE;
                     break;
                  } 
    return rcv;
}










































/*static gboolean draw_node(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    GdkRGBA color;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    color.red = 0.5;
    color.green = 0.5;
    color.blue = 0.5;
    color.alpha = 1.0;

    gdk_cairo_set_source_rgba(cr, &color);
    cairo_arc(cr, width / 2, height / 2, MIN(width, height) / 2 - 5, 0, 2 * G_PI);
    cairo_fill(cr);

    return FALSE;
}
static gboolean draw_arrow(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    GdkRGBA color;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);

    color.red = 0.0;
    color.green = 0.0;
    color.blue = 0.0;
    color.alpha = 1.0;

    gdk_cairo_set_source_rgba(cr, &color);
    cairo_move_to(cr, width / 2, 5);
    cairo_line_to(cr, 5, height - 5);
    cairo_line_to(cr, width - 5, height - 5);
    cairo_fill(cr);

    return FALSE;
}



void CreateNode(){
    node1 = gtk_drawing_area_new();
    gtk_widget_set_size_request(node1, 40, 40);
    g_signal_connect(G_OBJECT(node1), "draw", G_CALLBACK(draw_node_cb), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), node1, 50, 50);

}

void CreateArow(){
    arrow = gtk_drawing_area_new();
    gtk_widget_set_size_request(arrow, 20, 20);
    g_signal_connect(G_OBJECT(arrow), "draw", G_CALLBACK(draw_arrow_cb), NULL);
    gtk_fixed_put(GTK_FIXED(fixed), arrow, 200, 100);

}*/



//what if client send infos for a server thats not connected to main server?
//void DrowGraph(int i,sockaddr_in  ServerInfo){

//}