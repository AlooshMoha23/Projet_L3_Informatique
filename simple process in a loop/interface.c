#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <math.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <stdbool.h>





// to compile: bash compile.script

GtkWidget *App;
GtkWidget *container1;
GtkWidget *processus;
GtkBuilder *builder;// pointer unsed in connection with loading the xml file (interface.glade)

int cpt=1;

gboolean update_label_text(gpointer data) {
    gchar* text;
    if(cpt==1){
         text = "Etat du processus: active";
         cpt=0;
    }
    else{
        text = "Etat du processus: inactive";
        cpt=1;
    }
    gtk_label_set_text(GTK_LABEL(processus), text);

    return true;

}
int main(int argc, char *argv[]){

    gtk_init(&argc, &argv); //for any gtk paramtres passed int the command ligne 

//--------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------
//connection with the glade file

    builder=gtk_builder_new_from_file("interface.glade");

    App    = GTK_WIDGET(gtk_builder_get_object(builder, "App"));  //"APP" is the one from the interface

    g_signal_connect(App, "destroy", G_CALLBACK(gtk_main_quit), NULL); //when the window App is destroyed (when we leave the app)  call the callback gtk_main_quit
    gtk_builder_connect_signals(builder, NULL);

    container1 = GTK_WIDGET(gtk_builder_get_object(builder, "container1")); //connection with container1
    processus= GTK_WIDGET(gtk_builder_get_object(builder,"processus"));
  
    gtk_widget_show(App); //show the window

    
    
    g_timeout_add(2000, update_label_text, NULL);






    gtk_main(); //it will keep listenning for signals and callbacks
   

    return EXIT_SUCCESS;



}




















