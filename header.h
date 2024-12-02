#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {

	char* progname;
	int redirect[2];
	char* args[];

}cmd;

typedef struct {

	int commandCount;
	cmd* commands[];

}pipeline;

cmd* parseCommand(char * str);

pipeline* parsePipeline(char * str);
ssize_t promptInput(const char* prompt, char** line, size_t* len);
void freePipeline(int numberOfPipes, int(*pipes)[2]);
int execWrapper(cmd* command, int numberOfPipes, int (*pipes)[2]);
pid_t runRedirect(cmd* command, int numberOfPipes, int (*pipes)[2]);
/*
  Function Declarations for builtin shell commands:
 */
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_num_builtins();