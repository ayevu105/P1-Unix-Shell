/* @file shell.c
 * @brief The following code fufills the requirments of the Unix Shell project
 * where the main driver serves as a shell interface that accepts commands and then executes them in seperate processes. 
 * @author Anthony Vu
 * @date 01/18/2023
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 80 /* The maximum length command */

int main(void) {
    int status = 0;
    char *args[MAX_LINE / 2 + 1]; /* command line arguemnts */
    int should_run = 1; /* flag to determine when to exit program */

    char historyBuff[MAX_LINE + 1] = "No commands in history.\n";

    while (should_run) {
        printf("osh>");
        fflush(stdout);

        /** 
         * After reading user input, the steps are:
         * (1) fork a child process using for()
         * (2) the child process will invoke execvp()
         * (3) oarent will invoke wait() unless command included &
         */

        char input[MAX_LINE + 1];
        
        //saves user input
        fgets(input, MAX_LINE + 1, stdin);

        //exits shell and terminate
        if (strcmp(input,"exit\n") == 0) {
            should_run = 0;
        } else {
            //forks a child process using fork()
            int pid = fork();
            //child processes invokes execvp()
            if (pid == 0) { 
                char *tokens;
                if (strcmp(input,"!!\n") == 0) { 
                    printf("%s", historyBuff);
                    tokens = historyBuff;
                } else { 
                    tokens = input;
                    strcpy(historyBuff, input);
                }

                bool ampersand = false;
                int ampIndex;
                bool redirectOut = false;
                bool redirectIn = false;
                int redirectIndex; 
                bool hasPipe = false;
                int pipeIndex;

                tokens[strlen(tokens)-1] = 0;
                tokens = strtok(tokens, " ");

                //splits the string and inserts tokens to 'args'
                int i = 0;
                while (tokens) {
                    args[i] = tokens;
                    if (strcmp(args[i],"&") == 0) {
                        ampersand = true;
                        ampIndex = i;
                    } else if (strcmp(args[i],">") == 0) {
                        redirectOut = true;
                        redirectIndex = i;
                    } else if (strcmp(args[i],"<") == 0) {
                        redirectIn = true;
                        redirectIndex = i;
                    } else if (strcmp(args[i],"|") == 0) {
                        hasPipe = true;
                        args[i] = NULL;
                        pipeIndex = i;

                        //create pipe
                        int pipeFD[2];
                        pipe(pipeFD);
                        int pidPipe = fork();

                        //left
                        if (pidPipe == 0) {
                            close(pipeFD[0]);
                            dup2(pipeFD[1],STDOUT_FILENO);
                            execvp(args[0], args);
                        //right
                        } else {
                            close(pipeFD[1]);
                            dup2(pipeFD[0],STDIN_FILENO);
                            wait(NULL);
                        }
                    }
                    tokens = strtok (NULL, " ");
                    i++;
                }
                args[i] = NULL;
                //dont use '&' as argument
                if (ampersand) {
                    args[ampIndex] = NULL;
                }
                //redirects output
                if (redirectOut) {
                    args[redirectIndex] = NULL;
                    FILE *fp = fopen(args[redirectIndex + 1], "w");
                    int fd = fileno(fp);
                    dup2(fd,STDOUT_FILENO);
                    fclose(fp);

                //redirects input
                } else if (redirectIn) {
                    args[redirectIndex] = NULL;
                    FILE *fp = fopen(args[redirectIndex + 1], "r");
                    int fd = fileno(fp);
                    dup2(fd,STDIN_FILENO);
                    fclose(fp);
                } else if(hasPipe) {
                    execvp(args[pipeIndex + 1], args);
                }
                execvp(args[0], args);
                exit(status);
            } else {
                if (strcmp(input,"!!\n") != 0) {
                    strcpy(historyBuff, input);
                }

                //parent invokes wait() unless command included '&'
                if (input[strlen(input) - 2] != '&') {
                    wait(NULL);
                    sleep(1);
                }
            }
        }
    }
    return 0;
}
