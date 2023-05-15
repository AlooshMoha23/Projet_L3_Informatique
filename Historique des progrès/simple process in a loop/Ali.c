void on_StartButton_clicked(GtkButton *b){
     
    /* hide the start button */
    gtk_widget_hide(StartButton);

    /* create child process */
    pid_t child_pid = fork();

        /* check if the child process was created successfully */
        if (child_pid == 0) {
               while(1){ 
                 printf("child\n"); 
                 gtk_label_set_text(GTK_LABEL(processus), (const gchar *) "the child is active");
                 while(gtk_events_pending()){ 
	               gtk_main_iteration();}
                   usleep(900000);
               
               }
       
        } else {

            kill(child_pid, SIGSTOP);
            
            do { 
                  gtk_label_set_text(GTK_LABEL(processus), (const gchar *) "the child is sleeping");
                 // while(gtk_events_pending()){
	              //  gtk_main_iteration();}
            
                  printf("parent\n");
                  usleep(900000);
                  kill(child_pid, SIGCONT);
                  usleep(900000);
                  kill(child_pid, SIGSTOP);
            }while(1);      
        }
    }
--------------------------------------------------------------------

int fd[2];

void on_StartButton_clicked(GtkButton *b){
  // hide the start button 
  gtk_widget_hide(StartButton);

  // create child process 
  pid_t child_pid = fork();

  // check if the child process was created successfully 
  if (child_pid == 0) {
    close(fd[0]);
    int i = 0;
    while(1){ 
      printf("child\n"); 
      gtk_label_set_text(GTK_LABEL(processus), (const gchar *) "the child is active");
      usleep(900000);
      write(fd[1], &i, sizeof(int));
      i++;
    }
  } else { 
    close(fd[1]);
    int i = 0;
    do { 
      gtk_label_set_text(GTK_LABEL(processus), (const gchar *) "the child is sleeping");
      usleep(900000);
      read(fd[0], &i, sizeof(int));
      printf("parent: %d\n", i);
    } while(1);   
  }
}

-----------------------------------------------------------------------------------------------------------------

gboolean update_label(gpointer data) {
    gtk_label_set_text(GTK_LABEL(processus), (const gchar *) data);
    return FALSE;
}

void on_StartButton_clicked(GtkButton *b){
    gtk_widget_hide(StartButton);

    pid_t child_pid = fork();

    if (child_pid == 0) {
        while(1){
            printf("child\n");
            gdk_threads_add_idle(update_label, "the child is active");
            usleep(900000);
        }
    } else {
        kill(child_pid, SIGSTOP);

        do {
            printf("parent\n");
            gdk_threads_add_idle(update_label, "the child is sleeping");
            usleep(900000);
            kill(child_pid, SIGCONT);
            usleep(900000);
            kill(child_pid, SIGSTOP);
        }while(1);
    }
}

gdk_threads_init();
gdk_threads_enter();

--------------------------------------------------------------------------------------------------------------------------

pid_t  child_pid=-1;

gboolean update_label_text(gpointer data) {
    gchar* text;
    printf("hiiiiiiiiiii\n");
    if (kill(child_pid, 0) == 0) {
        text = "The child process is active";
       } 
    else {
        text = "The child process is stopped";
    }
    gtk_label_set_text(GTK_LABEL(processus), text);
   // while(gtk_events_pending()){ 
	   //            gtk_main_iteration();}

    return G_SOURCE_CONTINUE;
}

void on_StartButton_clicked(GtkButton *b){
    gtk_widget_hide(StartButton);

    child_pid = fork();

    if (child_pid == 0) {
        while(1){ 
        
            printf("child\n");
            g_timeout_add(2000, update_label_text, NULL);
            raise(SIGSTOP);
        }
    } else {
    
             int status;
             waitpid(child_pid, &status, WUNTRACED);  
        
             printf("parent\n");
             g_timeout_add(2000, update_label_text, NULL);
             kill(child_pid, SIGCONT);
             //sleep(1);
          
             
        
    }
}
   