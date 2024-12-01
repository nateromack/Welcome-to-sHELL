
#include "header.h"


char* nextToken(char**line){
    char * token;
    while((token=strsep(line," \t\n\r")&&!*token));
    return token;
}

cmd* parseCommand(char * str){
    char * copy = strndup(str,1024);
    char* token;
    int i=0;
    cmd* ret = calloc(sizeof(cmd) + 1024 * sizeof(char), 1);

    while(((token = nextToken(&copy)))){
        ret->args[i++] = token;
    }
    ret->progname = ret->args[0];
    ret->redirect[0] = -1;
    ret->redirect[1] = -1;
    return ret;
}
pipeline* parsePipeline(char * str){
    char * copy = strndup(str,1024);
    char* token;
    int numberCommands = 0;
    int i = 0;
    pipeline* ret;
    for(char*current = copy; *current; current++){
        if(*current == '|'){
            numberCommands++;
        }
    }
    ret = calloc(sizeof(pipeline) + numberCommands * sizeof(cmd), 1);
    ret->commandCount = numberCommands;

    while(token = strsep(&copy, "|")){
        ret->commands[i++] = parseCommand(token);
    }
    return ret;
}
