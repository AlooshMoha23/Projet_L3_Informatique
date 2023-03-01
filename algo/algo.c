#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int main(int argc, char *argv[]) {

  while (1) {
    pid_t pid = fork();
    if (pid == 0) {
      sleep(1);
      printf("etat 1\n");
      exit(0);

    } else {
      wait(0);
      sleep(1);
      printf("etat 0\n");
    }
  }
  return (0);
}