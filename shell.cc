#include <cstdio>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <signal.h>
#include <string.h>


#include "shell.hh"

int yyparse(void);
void yyrestart(FILE* file);

#define MAXFILENAME 1024

char* currentArg;


void Shell::prompt() {
	printf("myshell>");
  fflush(stdout);
}

extern "C" void sigHandler_ctrlC ( int sig ) {
	// if (sig == SIGINT) {
	// 	printf("\nEXITING PROGRAM\n");
	// 	exit(EXIT_SUCCESS);
	// }
  printf("\n");
  if ( isatty(0) ) {
    Shell::prompt();
  }
}

extern "C" void sigHandler_zombie ( int sig ) {
	pid_t pid = wait3(0, 0, NULL);
	while (waitpid(-1, NULL, WNOHANG) > 0); 
}


int main() {
  /*****     Zombie START      ******/
	// list_of_pid = (int*)(malloc(sizeof(int)*999));
	struct sigaction SIGINT_action_Zombie;
	SIGINT_action_Zombie.sa_handler = sigHandler_zombie;
	sigemptyset(&SIGINT_action_Zombie.sa_mask);
	SIGINT_action_Zombie.sa_flags = SA_RESTART;

	if (sigaction(SIGCHLD, &SIGINT_action_Zombie, NULL)) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	/*****     Zombie END      ******/

  /*****     Ctrl - C START      ******/
	struct sigaction SIGINT_action_CtrlC;
	SIGINT_action_CtrlC.sa_handler = sigHandler_ctrlC;
	sigemptyset(&SIGINT_action_CtrlC.sa_mask);
	SIGINT_action_CtrlC.sa_flags = SA_RESTART;

	if (sigaction(SIGINT, &SIGINT_action_CtrlC, NULL)) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
  /*****     Ctrl - C END      ******/

	// FILE *fd;
  FILE *fd = fopen(".shellrc", "r");
  if (fd)
  {
    yyrestart(fd);
    yyparse();
    yyrestart(stdin);
    fclose(fd);
  }else{
    if ( isatty(0) ) {
      Shell::prompt();
    }
  }
	
  yyparse();
}

Command Shell::_currentCommand;
