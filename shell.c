#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "header.h"

int main(int argc, char** argv){

    char * line = NULL;
    size_t len = 0;

    while(promptInput("Shell> ", &line, &len) > 0) {
        pipeline*  pipeline = parsePipeline(line);
        int numberOfPipes = pipeline->commandCount;
        int(*pipes)[2] = calloc (sizeof(int[2]), numberOfPipes);
        for(int i = 1; i < numberOfPipes; ++i) {
            pipe(pipes[i-1]);
            pipeline->commands[i]->redirect[STDIN_FILENO] = pipes[i-1][0];
            pipeline->commands[i]->redirect[STDOUT_FILENO] = pipes[i-1][1];
        }
        for(int i = 0; i < numberOfPipes; ++i) {
            runRedirect(pipeline -> commands[i], numberOfPipes, pipes);
        }
        freePipeline(numberOfPipes, pipes);

        for(int i = 0; i < numberOfPipes; ++i) {
            wait(NULL);
        }
        fputs("\n",stderr);
    }
        return 0;
}

ssize_t promptInput(const char* prompt, char** line, size_t* len) {
        fputs(prompt, stderr);
        return getline(line, len, stdin);
    }

    void freePipeline(int numberOfPipes, int(*pipes)[2]){
        for (int i; i < numberOfPipes; ++i) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
int execWrapper(cmd* command, int numberOfPipes, int (*pipes)[2]) {
    int fd = -1;
    if (command->redirect[0] != -1) {
        dup2(fd,STDIN_FILENO);
    }
    if (command->redirect[1] != -1) {
        dup2(fd, STDOUT_FILENO);
    }
    freePipeline(numberOfPipes, pipes);
    return execvp(command->progname, command->args);
}
pid_t runRedirect(cmd* command, int numberOfPipes, int (*pipes)[2]) {
    pid_t child = fork();
    if(child){
        switch(child){
            case -1:
                fprintf(stderr,"Child");
                return -1;
            default:
                return child;
        }
        }else{
            execWrapper(command,numberOfPipes, pipes);
            perror("Problem with command");
            return 0;
        }

    
}