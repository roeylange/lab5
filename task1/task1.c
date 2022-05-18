#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "LineParser.h"
#include <linux/limits.h>
#include <sys/wait.h>

void printPath();
void execute (cmdLine* p);
int debug = 0;

int main (int argc , char* argv[], char* envp[])
{
	
	char buf2[2048];
	for (int i=1; i<argc; i++){
		if(strcmp(argv[i], "-d")==0) debug=1;
	}
	while(1){
		printPath();
		fgets(buf2, 2048, stdin);
		cmdLine* commands = parseCmdLines(buf2);
		execute(commands);
		freeCmdLines(commands);
	}
  	return 0;
}

void printPath(){
	char buf1[PATH_MAX]; 
	char* currDir = getcwd(buf1, 2048);
	printf("current directory: %s\n", currDir);
}

void execute (cmdLine* p){
	int error = 0;
	if(strcmp(p->arguments[0],"quit")==0){
		_exit(EXIT_SUCCESS);
	}  else if(strcmp(p->arguments[0],"cd")==0){
		error = chdir(p->arguments[1]);
		if(error<0) {
			perror("error");
			exit(EXIT_FAILURE);
		}
		return;
	} 
	int pid = fork();
	if(!pid){	
		error = execvp(p->arguments[0], p->arguments);
		if(error<0) {
			perror("error");
			_exit(EXIT_FAILURE);
		}
  
	}
	if (debug){
			fprintf(stderr, "PID: %d\nExecuting command: %s\n",pid,p->arguments[0]);
		}
	if (p->blocking) {
		waitpid(pid,NULL,0);
	} 

      
	

}


