#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>

#define N 100

pid_t pid, child_pid;
void TarasBulba(int sig);

void main() {
	char path[N];
	char command[N];
	char *ptr;
  	int i;
	char sep[] = " ";
	char *words[N/2+1];
	bool mode;
	int status;
	
	while(1) {
		(void) signal(SIGINT, TarasBulba);
		if (!getcwd(path, N))
			strcpy(path, "..."); 
  		printf("%s->", path);
		if (!fgets(command, N, stdin))
			printf("Error! Too many symbols in the command!");
		command[strlen(command)-1] = '\0';
		ptr = strtok(command, sep);
		i = 0;
   		while (ptr) {
       			words[i++] = ptr;
       			ptr = strtok (NULL, sep);
     		}
	    	if ((i > 1) && (!strcmp(words[i-1], "$"))) {
			mode = 1;
			words[i-1] = NULL;
		} else {
			mode = 0;
			words[i] = NULL;
		}
	 	if (!strcmp(words[0], "quit"))
	     		exit(0);
	 	if (!strcmp(words[0], "cd")) { 
			chdir(words[1]);
	       		continue; 
	     	}
		pid = fork();
	  	switch(pid) {
	  		case -1:
		  		perror("Fork failed!");
		  		exit(1);
	  		case 0:
		  		printf(" CHILD: My PID is -- %d\n", getpid());
		  		printf(" CHILD: My parent PID is -- %d\n", getppid());
				execvp(words[0], words);
	  		default:
		  		printf("PARENT: My PID is -- %d\n", getpid());
		  		printf("PARENT: My child PID is -- %d\n\n", pid);
		  		if (!mode) {
		   	  		child_pid = pid;
				  	child_pid = wait(&status);
				  	printf("Child has finished: PID = %d\n", child_pid);
				  	if (WIFEXITED(status))
			 	  		printf("Child exited with code %d\n", WIFEXITED(status));
				  	else
				  		printf("Child terminated incorrectly!\n");
					child_pid = 0;
		  		}
				break;
	   
	  	}
	}
}

void TarasBulba(int sig) {
	if (child_pid) {
		//printf("\nYa tebya porodil, ya tebya i ub`u!\n");
		kill(child_pid, SIGINT);
	}
}
