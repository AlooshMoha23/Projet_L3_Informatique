#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>





void *run_gtk_main(void *arg) {//first thread that would execute in pararell
    gtk_main();
    return NULL;
}




/*******************t***************************************************/
/*drawing                                    */
/*****************************************************************************/

static void draw_node(cairo_t *cr, int x, int y, int etat)
{

    
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_arc(cr, x, y, 10, 0, 2 * G_PI);//NODE_SIZE=10
    cairo_fill(cr);
}

 static gboolean update_node(GtkWidget *widget, cairo_t *cr, gpointer data){

   
    if(color==0){
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);

    }
     if(color==1){
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);

    }

    return FALSE;

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
   // srand(time(NULL));

  //  for (i = 0; i < 1; i++) {
        x[i] = (i + 1) * (800 / (n + 1));  //800=WIDTH
        y[i] = (rand() % (600 - 50)) + 50 / 2; //50 = NODE_SPACING   600=HEIGHT
        draw_node(cr, x[i], y[i],n);
   // }

    /*for (i = 0; i < n; i++) {
        j = (i + 1) % n;
        k = (i + 2) % n;
        draw_line(cr, x[i], y[i], x[j], y[j]);
        draw_line(cr, x[i], y[i], x[k], y[k]);
    }*/

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




void *GTK(GtkWidget *App,GtkWidget *Container,GtkBuilder *builder){
   

    builder=gtk_builder_new_from_file("interface.glade");
    App = GTK_WIDGET(gtk_builder_get_object(builder, "App")); //"APP" is the one from the interface
    Container = GTK_WIDGET(gtk_builder_get_object(builder, "Container"));

    g_signal_connect(App, "destroy", G_CALLBACK(gtk_main_quit), NULL); //when the window App is destroyed (when we leave the app)  call the callback gtk_main_quit
    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(G_OBJECT(Container), "draw", G_CALLBACK(draw_cb), NULL);

    
    gtk_widget_show(App); //show the window
    

    return NULL;
}
































