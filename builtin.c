#include "header.h"

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit
};


int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}


int sh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int sh_help(char **args)
{
  int i;
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < sh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int sh_exit(char **args)
{
  exit(0);
  return(0);
}