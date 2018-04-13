#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <ctype.h>

#include "sfish.h"
//#include "debug.h"
#include "helper.h"

pid_t pid;
int status;
char* CurrentCmd;
int jobID;
struct job jobs[1000];
int no_input(char *input) {
  while (*input != '\0') {
    if (!isspace((unsigned char)*input))
      return 0;
    input++;
  }
  return 1;
}

int built_check(char* cmd){
    if(strcmp(cmd,"help") == 0){
        return 0;
    }
    else if(strcmp(cmd, "pwd") == 0){
        return 0;
    }
    return 1;
}
void run_builtin(char* cmd){
        pid = fork();
        if(pid < 0){
            printf(EXEC_ERROR, "Fork Failed");
            exit(-1);
        }
        if(pid == 0){
            //Help
            if(strcmp(cmd,"help") == 0){
            printf("List of Builtin commands");
            printf("help: Prints a list of all builtins and their usage\n");
            printf("exit: Exits the Shell\n");
            printf("cd: Change current working directory\n");
            printf("pwd: Prints absolute path of current working directory\n");
            }
            //PWD
            else if(strcmp(cmd, "pwd") == 0)
            {
                char cwd[PATH_MAX];
                getcwd(cwd,PATH_MAX);
                printf("%s\n",cwd);
            }
        }
        else{
            pause();
        }
}

void run_simplecommand(char **cmd){
    //debug("%s",*cmd);
    char *copy = malloc(strlen(*cmd) + 1);
    strcpy(copy, *cmd);
    jobs[jobID].JobID = jobID;
    jobs[jobID].command = copy;
    pid = fork();
    jobs[jobID].pid = pid;
    jobID++;
    if(pid < 0){
        printf(EXEC_ERROR, "Fork Failed");
        exit(-1);
    }
    else if(pid == 0){
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
            if(execvp((*cmd), cmd) == -1){
                printf(EXEC_NOT_FOUND, (*cmd));
                exit(EXIT_FAILURE);
            }
        }
    else{
        //setpgid(pid, pid);
        //tcsetpgrp(STDIN_FILENO, getpid());
        //waitpid(pid, NULL, WUNTRACED);
        pause();
    }
}

int redirection(char *args, int re){//re = 0 for left, 1 for right
    int fd;
    if(re == 0){
    fd = open(args,O_RDONLY);
    if(fd != -1){
        dup2(fd,STDIN_FILENO);
        return 0;
    }
    }
    else if(re == 1){
    fd = open(args, O_CREAT | O_WRONLY, 0777);
    if(fd != -1){
        dup2(fd,STDOUT_FILENO);
        return 0;
    }
    }
    return 1;
}


void run_redirection(char *input){
    char *reCommands[strlen(input)];
    char *argList[strlen(input)];
    int reCount = -1;
    int argCount = -1;

    int indexofLeft = strchr(input,'<')- input;
    int indexofRight = strchr(input,'>')- input;

    int Leftcount = 0;
    int Rightcount = 0;

    for (int i =0; input[i]; i++){
    Leftcount += (input[i] == '<');
    }

    for (int i =0; input[i]; i++){
    Rightcount += (input[i] == '>');
    }

    if(indexofLeft == 0 || indexofRight == 0){
        printf(SYNTAX_ERROR,"No Args, Input starts with < or >");
    }
    else if(Leftcount > 1 || Rightcount > 1){
        printf(SYNTAX_ERROR,"Multiple > or Multiple <");
    }
    else{
    //debug("LeftIndex %d",indexofLeft);
    //debug("RightIndex %d",indexofRight);

    char *temp = strtok(input, "<>");
    while(temp != NULL){
            if(strlen(temp) > 0)
            {
                reCommands[++reCount] = temp;
                //debug("Commands: %s", reCommands[reCount]);
            }
            temp = strtok(NULL,"<>");
        }


    argCount = -1;
    temp = strtok(reCommands[0], " ");
    while(temp != NULL){
            if(strlen(temp) > 0)
            {
                argList[++argCount] = temp;
                //debug("Args: %s", argList[argCount]);
            }
            temp = strtok(NULL," ");
    }
    argList[++argCount] = NULL;

    if(reCount > 1){
    reCommands[2] = strtok(reCommands[2], " ");
    }
    reCommands[1] = strtok(reCommands[1], " ");

    pid = fork();
    if(pid < 0){
        printf(EXEC_ERROR, "Fork Failed");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        //If only left redirection
        if(indexofLeft > indexofRight && indexofRight < 0){
            int fd = open(reCommands[1],O_RDONLY);
            if(fd != -1){
                dup2(fd,STDIN_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
        }
        //If only Right redirection
        else if(indexofRight > indexofLeft && indexofLeft < 0){
            int fd = open(reCommands[1], O_CREAT | O_WRONLY, 0777);
            if(fd != -1){
                dup2(fd,STDOUT_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
        }
        //Left then Right
        else if(indexofLeft < indexofRight){
            int fd = open(reCommands[1], O_RDONLY);
            if(fd != -1){
                dup2(fd,STDIN_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
            fd = open(reCommands[2],O_CREAT | O_WRONLY, 0777);
            if(fd != -1){
                dup2(fd,STDOUT_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
        }
                //Right then Left
        else if(indexofRight < indexofLeft){
            int fd = open(reCommands[1], O_CREAT | O_WRONLY, 0777);
            if(fd != -1){
                dup2(fd,STDOUT_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
            fd = open(reCommands[2],O_RDONLY);
            if(fd != -1){
                dup2(fd,STDIN_FILENO);
            }
            else{
                printf(SYNTAX_ERROR, "Invalid File");
                exit(EXIT_FAILURE);
            }
        }
        else{
            printf(SYNTAX_ERROR,"Redirection Failed");
            exit(EXIT_FAILURE);
        }
        int bc = built_check(argList[0]);
            if(bc == 1){
            if(execvp(argList[0], argList) == -1){
                printf(EXEC_NOT_FOUND, argList[0]);
                exit(EXIT_FAILURE);
            }
            }
            else{
                exit(EXIT_SUCCESS);
            }
    }
    else{
        pause();
    }
}
}

void run_pipe_command(char* input){
    char *pipeCommands[strlen(input)];
    char *argList[strlen(input)];
    int pipesCount = -1;
    int argCount = -1;

    char *temp = strtok(input, "|");
    while(temp != NULL){
            if(strlen(temp) > 0)
            {
                pipeCommands[++pipesCount] = temp;
            }
            temp = strtok(NULL,"|");
        }
    argList[++pipesCount] = NULL;

    for(int j = 0; j < pipesCount; j++){
    argCount = -1;
    temp = strtok(pipeCommands[j], " ");
    while(temp != NULL){
            if(strlen(temp) > 0)
            {
                argList[++argCount] = temp;
            }
            temp = strtok(NULL," ");
    }
    argList[++argCount] = NULL;

    int SavedPipes[2];

    int pipe_1[2];
    if(j < pipesCount-1)
        {
        pipe(pipe_1);
        }
    pid = fork();
    if(pid < 0){
        printf(EXEC_ERROR, "Fork Failed");
        exit(-1);
    }
    if(pid == 0){
        if(j > 0)
            {
                close(SavedPipes[1]);
                dup2(SavedPipes[0], 0);
            }

        else
            {
                close(pipe_1[0]);
                dup2(pipe_1[1], 1);
            }
        if(execvp(argList[0], argList) == -1){
                printf(EXEC_NOT_FOUND, argList[0]);
                exit(EXIT_FAILURE);
            }
    }
    else{
            if(j > 0)
            {
                close(SavedPipes[0]);
                close(SavedPipes[1]);
            }

            else
            {
                SavedPipes[0] = pipe_1[0];
                SavedPipes[1] = pipe_1[1];
            }

    }
}
pause();
}

void sig_interrupt(int signo) {
}

void sig_suspend(int signo) {
}

void sig_child(int signo) {
    waitpid(pid,&status,WUNTRACED);
    if(WIFSTOPPED(status)){
        for(int i = 0; i < jobID; i++){
            if(pid == jobs[i].pid){
                jobs[i].Stopped = 1;
            }
            //debug("%d",pid);
            //kill(pid, SIGCONT);
        }
    }
}
