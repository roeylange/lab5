#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "LineParser.h"
#include <linux/limits.h>

void printPath();
void execute (cmdLine* p);


int main (int argc , char* argv[], char* envp[])
{
	
	char buf2[2048];
	while(1){
		printPath();     /*print the directory path*/
		fgets(buf2, 2048, stdin);   /*read a command from user*/
		cmdLine* commands = parseCmdLines(buf2);   /*parse the command*/
		execute(commands); /*executes*/
		freeCmdLines(commands);  /*free*/
	}
	
  return 0;
}

void printPath(){
	char buf1[PATH_MAX]; 
	char* currDir = getcwd(buf1, 2048);
	printf("current directory: %s\n", currDir);
}

void execute (cmdLine* p){
	if(strcmp(p->arguments[0], "quit")==0){
		exit(0);
	}  
	int error = execvp(p->arguments[0], p->arguments);
	if(error<0) {
		perror("error");
		exit(EXIT_FAILURE);
	}

}


