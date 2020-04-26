#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void
open_pipe_or_exit(int *fds)
{
    if (pipe(fds) != 0) {
        printf("pipe open failure\n");
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    int child_to_parent_fds[2];
    int parent_to_child_fds[2];
    open_pipe_or_exit(child_to_parent_fds);
    open_pipe_or_exit(parent_to_child_fds);

    int child_pid = fork();
    if (child_pid < 0) {
        printf("fork failure\n");
        exit(1);
    }

    char value;
    if (child_pid) {
        close(child_to_parent_fds[1]);
        close(parent_to_child_fds[0]);

        if (write(parent_to_child_fds[1], "a", 1) <= 0) {
            printf("fail\n");
            exit(0);
        }
        if (read(child_to_parent_fds[0], &value, 1) == 1) {
            int my_pid = getpid();
            printf("%d: received pong\n", my_pid);
        }

        close(parent_to_child_fds[1]);
        close(child_to_parent_fds[0]);
    } else {
        close(child_to_parent_fds[0]);
        close(parent_to_child_fds[1]);

        if (read(parent_to_child_fds[0], &value, 1) == 1) {
            printf("%d: received ping\n", child_pid);
        }

        if (write(child_to_parent_fds[1], "a", 1) <= 0) {
            printf("fail\n");
            exit(0);
        }

        close(parent_to_child_fds[0]);
        close(child_to_parent_fds[1]);
    }


    exit(0);
}
