#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sfish.h"
#include "helper.h"


int main(int argc, char *argv[], char* envp[]) {
    char *DefaultColor = KNRM;
    char *ChosenColor = KNRM;
    jobID = 0;
    char* input;
    bool exited = false;
    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }
    signal(SIGINT, sig_interrupt);
    signal(SIGTSTP, sig_suspend);
    signal(SIGCHLD, sig_child);

    do {
        char cwd[100];
        getcwd(cwd,100);
        char* Coloredcwd = malloc(strlen(cwd) + strlen(ChosenColor) + 1);
        strcpy(Coloredcwd, ChosenColor);
        strcat(Coloredcwd, cwd);
        //Setup Prompt
        int size = strlen(Coloredcwd) + strlen(DefaultColor) + strlen(" :: zowwang >>") + 1;
        char *prompt = malloc(sizeof(char)*size + 1);
        strcpy(prompt, Coloredcwd);
        strcat(prompt," :: zowwang >>");
        strcat(prompt, DefaultColor);
        //Readline
        printf("%s",prompt);
        input = readline("");
        free(prompt);
        /*write(1, "\e[s", strlen("\e[s"));
        write(1, "\e[20;10H", strlen("\e[20;10H"));
        write(1, "SomeText", strlen("SomeText"));
        write(1, "\e[u", strlen("\e[u"));*/

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }

        if(no_input(input) == 1){
            printf(EXEC_NOT_FOUND, "");
        }
        else if(strpbrk(input,"|") != NULL){
        run_pipe_command(input);
        }

        else if(strpbrk(input,"<") != NULL || strpbrk(input,">") != NULL){
        run_redirection(input);
        }
        else{
        char *args = strtok(input, " ");
        //Build-in Commands
        //Exit
        if(strcmp(args, "exit") == 0)
            {
            exit(0);
            }
        else if(strcmp(args, "color") == 0)
            {
                args = strtok(NULL," ");
                if(args != NULL){
                    if(strcmp(args,"RED") == 0){
                        ChosenColor = KRED;
                    }
                    else if(strcmp(args,"GRN") == 0){
                        ChosenColor = KGRN;
                    }
                    else if(strcmp(args,"YEL") == 0){
                        ChosenColor = KYEL;
                    }
                    else if(strcmp(args,"BLU") == 0){
                        ChosenColor = KBLU;
                    }
                    else if(strcmp(args,"MAG") == 0){
                        ChosenColor = KMAG;
                    }
                    else if(strcmp(args,"CYN") == 0){
                        ChosenColor = KCYN;
                    }
                    else if(strcmp(args,"WHT") == 0){
                        ChosenColor = KWHT;
                    }
                    else if(strcmp(args,"BWN") == 0){
                        ChosenColor = KBWN;
                    }
                    else{
                        printf(SYNTAX_ERROR, "Invalid Color");
                    }
                }
                else{
                    printf(SYNTAX_ERROR, "No Args for color");
                }
            }
        else if(strcmp(args, "jobs") == 0)
            {
            for(int i = 0; i < jobID; i++){
                if(jobs[i].Stopped == 1){
                printf(JOBS_LIST_ITEM, jobs[i].JobID, jobs[i].command);
                printf("PID:%d\n",jobs[i].pid);
                printf("Stopped:%d\n",jobs[i].Stopped);
            }
            }
            }
        else if(strcmp(args, "fg") == 0)
            {
            args = strtok(NULL," ");
            if(args != NULL){
            if(strncmp(args,"%",1) == 0){
            char* JID = strtok(args,"%");
            for(int i = 0; i < jobID; i++){
                if(jobs[i].Stopped == 1 && jobs[i].JobID == atoi(JID)){
                jobs[i].Stopped = 0;
                printf("\n");
                kill(jobs[i].pid, SIGCONT);
            }
            }
            }
            else{
            printf(SYNTAX_ERROR,"Invalid args for fg");
            }
            }
            else{
            printf(SYNTAX_ERROR,"Invalid args for fg");
            }
            }
        else if(strcmp(args, "kill") == 0)
            {
            args = strtok(NULL," ");
            if(args != NULL){
            if(strncmp(args,"%",1) == 0){
            char* JID = strtok(args,"%");
            for(int i = 0; i < jobID; i++){
                if(jobs[i].Stopped == 1 && jobs[i].JobID == atoi(JID)){
                jobs[i].Stopped = 0;
                kill(jobs[i].pid, SIGKILL);
                }
            }
            }
            else{
            for(int i = 0; i < jobID; i++){
                if(jobs[i].Stopped == 1 && jobs[i].pid == atoi(args)){
                jobs[i].Stopped = 0;
                kill(jobs[i].pid, SIGKILL);
                break;
                }
            }
            }
            }
            else{
            printf(SYNTAX_ERROR,"Invalid args for fg");
            }
            }
        //Cd
        else if(strcmp(args, "cd") == 0){
            char *prev;
            char *temp;
            char cwd[PATH_MAX];
            prev = getcwd(cwd,PATH_MAX);
            temp = getcwd(cwd,PATH_MAX);
            //Get Current Working Directory
            args = strtok(NULL," ");
            if(args == NULL){
            prev = getcwd(prev,PATH_MAX);
            chdir(getenv("HOME"));
            }
            else if(strcmp(args, "-") == 0){
            temp = getcwd(temp,PATH_MAX);
            chdir(prev);
            prev = strcpy(prev,temp);
            }
            else if(strcmp(args, ".") == 0){
            prev = getcwd(prev,PATH_MAX);
            chdir(cwd);
            }
            else if(strcmp(args, "..") == 0){
            prev = getcwd(prev,PATH_MAX);
            chdir("..");
            }
            else{
            prev = getcwd(prev,PATH_MAX);
            chdir(args);
            }
        }
        else{
        int bc = built_check(args);
        if (bc == 1){
            char *cmd[_POSIX_ARG_MAX];
            int i = 0;

            while(args != NULL){
            cmd[i] = args;
            i++;
            args = strtok(NULL," ");
            }
            cmd[i] = NULL;
        run_simplecommand(cmd);
        }
    }
    }

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

    } while(!exited);

    //debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}
