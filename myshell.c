// Steven Montalbano
// smm285

// gcc --std=c99 -Wall -Werror -o myshell myshell.c
// ./myshell

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int DEBUG_MODE = 0;

void showBanner() {
  FILE* image = fopen("testa.txt", "r");

  if(image == NULL)
    return;

  char c = fgetc(image);
  while (c != EOF)
    {
      printf ("%c", c);
      c = fgetc(image);
    }
  fclose(image);
}

void showHelp(){
  printf("Hello! Welcome to Testa, a shell interface capable of performing \nmost basic commands you would find on any Unix platform.\n");
  printf("Currently supported commands:\n");
  printf("cd - change directory \npwd - display current working directory \nexit X - exit Testa with error code X \n./FILE - run the executable FILE, if possible \n");
  printf("Output Redirection: COMMAND > OUTFILE.txt \nInput Redirection: COMMAND < INFILE.txt \n");
}

void redirectOutput(char** tokens, int index) {
  FILE* outfile = freopen(tokens[index + 1], "w", stdout);

  if (outfile == NULL){
    perror("Unable to open file for output redirection.");
    exit(errno);
  }
  tokens[index] = NULL;
}

void redirectInput(char** tokens, int index) {
  FILE* infile = freopen(tokens[index + 1], "r", stdin);

  if (infile == NULL){
    perror("Unable to open file for input redirection.");
    exit(errno);
  }
  tokens[index] = NULL;
}

int main(int argc, char** argv) {

  int cmd_num = 1;
  char buffer[350];   // user input buffer to hold cmd and arguments
  char* token;
  char* tokens[350/2];
  const char* DELIM = " \t\n";

  signal(SIGINT, SIG_IGN);	//ignore the ctr+C interrupt signal

  showBanner();   // displays the testa banner on launch

  while(1) {  // infinite loop to maintain the shell interface

    printf("(%d) testa $ ", cmd_num); // shell prompt
    fgets(buffer, 350, stdin);        // get user command

    token = strtok(buffer, DELIM);  // get first token, sets strtok global var to the buffer
    int i;
    for (i = 0; token != NULL; i++) {
      tokens[i] = token;
      token = strtok(NULL, DELIM);  // NULL as source uses the next token from the global var buffer
      if (DEBUG_MODE) printf("%d %s\n", i, token);
    }
    tokens[i] = NULL;               // set first invalid index to NULL

    if(tokens[0] == 0)
      continue; // return pressed, no command given

    // check for built ins before forking child process

    if (strcmp(tokens[0], "help") == 0)
      showHelp();

    else if (strcmp(tokens[0], "exit") == 0) {

      if(tokens[1] != 0)     // user specified an exit code
        exit(atoi(tokens[1]));
      else
        exit(0);
    }

    else if (strcmp(tokens[0], "cd") == 0)
      chdir(tokens[1]);

    else if (strcmp(tokens[0], "pwd") == 0)
      printf("%s\n", get_current_dir_name());

    else {
      if (fork() == 0) {  // The Shell process successfully forked

        signal(SIGINT, SIG_DFL);	//restore the ctr+C interrupt signal to exit child process back to the Testa shell

        int i = 0, input = 0, output = 0;   // Check for I/O Redirection
        while (tokens[i] != NULL) {
          if(strcmp(tokens[i], "<") == 0) { // redirect input
              if(input == 0) {
                redirectInput(tokens, i);
                input = 1;
              } else {
                perror("stdin can only be redirected once\n");
                exit(1);
              }
              continue;
            }

          if(strcmp(tokens[i], ">") == 0) { // redirect output
              if(output == 0) {
                redirectOutput(tokens, i);
                output = 1;
              } else {
                perror("stdout can only be redirected once\n");
                exit(1);
              }
              continue;
            }
          i++;
        }

        execvp(tokens[0], &tokens[0]);  // swap the child process memory from command in tokens
        fclose(stdout);
        fclose(stdin);
        exit(1);
      } else {  // Fork process encountered an error, return to parent process
        int status;
        int childpid = waitpid(-1, &status, 0);

        if (childpid == -1) {
          perror("Error Detected");
          exit(1);
        }
        else if(WIFSIGNALED(status) && strcmp("Interrupt", strsignal(WTERMSIG(status)))==0)
          printf("\nCtr C Detected - Exiting Process...\n");
        else if (!WIFEXITED(status))
          printf("Process Exited: %s\n", strsignal(WIFEXITED(status)));
      }
    }

    cmd_num++;
  } // end of while

  return 1; //if this is reached, the infinite while loop exit for some reason and an error occured
} //end of main
