#ifndef HELPER_H
#define HELPER_H

#define KNRM "\033[0m"
#define KRED "\033[1;31m"
#define KGRN "\033[1;32m"
#define KYEL "\033[1;33m"
#define KBLU "\033[1;34m"
#define KMAG "\033[1;35m"
#define KCYN "\033[1;36m"
#define KWHT "\033[1;37m"
#define KBWN "\033[0;33m"

int no_input(char *input);
int built_check(char* cmd);
void run_builtin(char* cmd);
void run_simplecommand(char **cmd);
void run_redirection(char* input);
void run_pipe_command(char* input);

int redirection(char *args, int re);
int process(char *args, char *special);

void sig_interrupt(int signo);
void sig_suspend(int signo);
void sig_child(int signo);

struct job
{
   pid_t pid;
   char* command;
   int JobID;
   int Stopped;
};
extern int jobID;
extern struct job jobs[1000];
#endif

