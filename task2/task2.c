#include "LineParser.h" 
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 2048

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
    cmdLine* cmd;                   /* the parsed command line*/
    pid_t pid; 		                /* the process id that is running the command*/
    int status;                     /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    time_t time;
    struct process *next;	        /* next process in chain */
} process;

process* TotalProcsList;

process* construct_Process(cmdLine* cmd, pid_t pid);
process* list_append(process* _process, cmdLine* cmd, pid_t pid);
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
char* getStatus(int status);
void printProc(process* process);
void list_print(process* process_list);
void deleteOneProc(process* toDelete);
void updateProcessList(process **);
void updateProcessStatus(process*,int,int);
int deleteTerPro(process** process_list);

process* construct_Process(cmdLine* cmd, pid_t pid){
    process* new_process = malloc(sizeof(struct process));
    new_process->cmd=cmd;
    new_process->pid=pid;
    new_process->status=RUNNING;
    time(&(new_process->time));
    new_process->next=NULL;
    return new_process;
}

process* list_append(process* _process, cmdLine* cmd, pid_t pid){
    process* new_process = construct_Process(cmd,pid);
    new_process->next = _process;
    return new_process;
}
void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
  (*process_list)=list_append((*process_list),cmd, pid);
}

void printProc(process* process){
    char command[100]="";
    time_t now;
    if(process!=NULL && process->cmd->argCount>0)
        for(int i=0;i<process->cmd->argCount;i++){
            const char* argument = process->cmd->arguments[i];
            strcat(command,argument);
            strcat(command," ");
        }
    printf("%d\t\t%s\t%s\t\t%ld\n",process->pid,command,getStatus(process->status),time(now)-process->time);
}

char* getStatus(int status){
    if(status == TERMINATED)
      return "Terminated";
    else if(status == RUNNING)
      return "Running";
    else          /*if(status == SUSPENDED)*/
      return "Suspended";
}

void list_print(process* process_list){
    process* curr_process = process_list;
    while(curr_process != NULL){
        printProc(curr_process);
        curr_process=curr_process->next;
    }
}

void printProcsList(process** process_list){
    updateProcessList(process_list);
    printf("PID\t\tCommand\t\tSTATUS\t\tAlive\n");
    list_print((*process_list));
    while(deleteTerPro(process_list)){};
}

void deleteOneProc(process* toDelete){
    freeCmdLines(toDelete->cmd);
    toDelete->cmd=NULL;
    toDelete->next=NULL;
    free(toDelete);
    toDelete=NULL;
}

int deleteTerPro(process** process_list){
    process* curr =  *process_list;
    process* prev;
    if(curr!=NULL && curr->status==TERMINATED){
        *process_list=curr->next;
        deleteOneProc(curr);
        return 1;
    }
    while (curr!=NULL && curr->status!=TERMINATED){
        prev=curr;
        curr=curr->next;
    }
    if(curr==NULL)
        return 0;
    else{
        prev->next=curr->next;
        deleteOneProc(curr);
        return 1;
    }
}

void freeProcessList(process* process_list){
    process* curr=process_list;
    if(curr!=NULL){
        freeProcessList(curr->next);
        freeCmdLines(curr->cmd);
        free(curr->cmd);
        free(curr);
    }
}

void updateProcessList(process **process_list){
    process* curr_process = (*process_list);
    while(curr_process!=NULL){
        int status;
        int wait = waitpid(curr_process->pid,&status,WNOHANG | WUNTRACED | WCONTINUED);
        if(wait!=0){    //status changed
            updateProcessStatus(curr_process,curr_process->pid,status);
        }
        curr_process=curr_process->next;
    }
}

void printPath(){
  char path_name[PATH_MAX];
  getcwd(path_name,PATH_MAX);
  fprintf(stdout, "%s\n",path_name);
}

void updateProcessStatus(process* process_list, int pid, int status){
    int tmpStatus=RUNNING;
    if(WIFEXITED(status) || WIFSIGNALED(status))
        tmpStatus=TERMINATED;
    else if(WIFSTOPPED(status))
        tmpStatus=SUSPENDED;
    else if(WIFCONTINUED(status))
        tmpStatus=RUNNING;
    process_list->status=tmpStatus;
}

int checkCommand(cmdLine* command){
  int checker = 0;
  if(strcmp(command->arguments[0],"quit")==0){
      checker = 1;
    _exit(EXIT_SUCCESS);
  }
  else if(strcmp(command->arguments[0],"cd")==0){
      checker=1;
    if(chdir(command->arguments[1])<0)
      perror("bad cd command");
  }
  else if(strcmp(command->arguments[0],"showprocs")==0){
      checker=1;
    printProcsList(&TotalProcsList);
  }
  else if(strcmp(command->arguments[0],"stop")==0){
      checker=1;
    int pid = atoi(command->arguments[1]);
    if(kill(pid,SIGINT)==-1)    //terminate
      perror(strerror(errno));
    else
      printf("%d handling SIGINT\n",pid);
  }
  else if(strcmp(command->arguments[0],"nap")==0){
      checker=1;
    int time = atoi(command->arguments[1]);
    int pid = atoi(command->arguments[2]);
      int suspend_fork;
      int kill_success;
      if ((suspend_fork = fork()) == 0){
        kill_success = kill(pid, SIGTSTP);
        if (kill_success == -1)
          perror("kill failed");
        else{
          printf("%d handling SIGTSTP\n",pid);
          sleep(time);
          kill_success = kill(pid, SIGCONT);
          if (kill_success == -1)
            perror("kill failed");
          else
            printf("%d handling SIGCONT\n",pid);
        }
        _exit(1);
      }
  }
  if(checker)
    freeCmdLines(command);
  return checker;
}

void execute(cmdLine* pCmdLine, int debugM){
  if(!checkCommand(pCmdLine)){
    pid_t childPid;
    if(!(childPid=fork())){
        if(execvp(pCmdLine->arguments[0],pCmdLine->arguments)<0){
          perror("can't execute the command");
          _exit(EXIT_FAILURE);
      }
    }
    if(childPid!=-1)  //child success
      addProcess(&TotalProcsList,pCmdLine,childPid);
    if(debugM){
      fprintf(stderr, "PID: %d\nExecuting command: %s\n",childPid,pCmdLine->arguments[0]);
    }
    if(pCmdLine->blocking){   //& ? 1 : 0
      waitpid(childPid,NULL,0);
    }
  }
}

int main(int argc, char const *argv[]) {
  FILE* input = stdin;
  char buf[BUFFER_SIZE];
  int debugM=0;
    TotalProcsList=NULL;
  for(int i=1;i<argc;i++){
    if((strcmp("-d",argv[i])==0)){
      debugM=1;
    }
  }
  while(1){
    printPath();
    fgets(buf,BUFFER_SIZE,input);
    cmdLine* line = parseCmdLines(buf);
    execute(line,debugM);
  }
  return 0;
}
