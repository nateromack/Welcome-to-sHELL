#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syst/wait.h>
#include <unistd.h>

typedef struct {

	char* progname;
	int redirect[2];

}cmd;

typedef struct {

	int cmdcount;
	cmd* cmds[]

} pipeline;

