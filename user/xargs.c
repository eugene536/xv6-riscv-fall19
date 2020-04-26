#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
fork_or_exit(void)
{
    int child_pid = fork();
    if (child_pid < 0) {
        printf("fork failure\n");
        exit(1);
    }
    return child_pid;
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
      fprintf(2, "usage: xargx command args...\n");
      exit(1);
    }

    static char *new_argv[MAXARG + 1];
    memmove(new_argv, argv + 1, (argc - 1) * sizeof(char *));
    new_argv[argc] = NULL;

    static char one_line[512];
    while (1) {
        gets(one_line, sizeof(one_line));
        int one_line_length = strlen(one_line);
        if (one_line_length == 0) {
            break;
        }

        if (fork_or_exit()) {
            wait(NULL);
        } else {
            one_line[one_line_length - 1] = '\0';
            new_argv[argc - 1] = one_line;
            exec(new_argv[0], new_argv);
        }
    }

    exit(0);
}
