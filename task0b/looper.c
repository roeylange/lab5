#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int signum);

int main(int argc, char **argv){ 

	printf("Starting the program\n");
	/*setting the handler to handle these 3 signals */
	signal(SIGTSTP, handler);
	signal(SIGCONT, handler);
	signal(SIGINT, handler);


	while(1) {
		sleep(2);
	}

	return 0;
}

void handler(int signum){
	printf("signal was received: %s\n", strsignal(signum));
	if(signum == SIGTSTP){printf("Stoping the process\n");}
	if(signum == SIGCONT){printf("Continuing the process\n");}
	if(signum == SIGINT){printf("Interrupt the process\n");}
	signal(signum,SIG_DFL);   /*handling the singal with the default handler*/
	raise(signum);    /*get again the signal*/
	if(signum == SIGTSTP){signal(SIGCONT,handler);}
	if(signum == SIGCONT){signal(SIGTSTP,handler);}
}
