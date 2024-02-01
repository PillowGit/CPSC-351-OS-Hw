#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {

  // Create variables to store the child and grandchild processes
  pid_t child_pid, grandchild_pid;
  // Create the child process
  child_pid = fork();

  // If making the process failed, print the error, otherwise do things w/ the
  // child process
  if (child_pid < 0) {
    fprintf(stderr, "Failed to create child process\n");
    return 1;
  } else if (child_pid == 0) {
    // Print the child process
    printf("Child process created with PID: %d\n", getpid());
    // Create the grandchild process now
    grandchild_pid = fork();

    // If grandchild failed to create blah blah same as before
    if (grandchild_pid < 0) {

      fprintf(stderr, "Failed to create grandchild process\n");
      return 1;

    } else if (grandchild_pid == 0) {

      // This all runs if grandchild process was created
      // print the grandchild process
      printf("Grandchild process created with PID: %d\n", getpid());
      // Replace grandchild process with Firefox
      char *args[] = {"firefox", NULL};
      execvp("firefox", args);
      // If we get here that means execvp failed so we will print an error
      fprintf(stderr, "Failed to start Firefox\n");
      return 1;

    } else {
      // In child process - wait for grandchild to finish
      wait(NULL);
      printf("Grandchild process terminated\n");
    }
  } else {
    // In main process - wait for child to finish
    wait(NULL);
    printf("Child process terminated\n");
  }

  return 0;
}
