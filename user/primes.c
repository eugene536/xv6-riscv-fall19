#include "kernel/types.h"
#include "user/user.h"

void
open_pipe_or_exit(int *fds)
{
    if (pipe(fds) != 0) {
        printf("pipe open failure\n");
        exit(1);
    }
}

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

void
write_int_or_exit(int *fds, int x)
{
    if (write(fds[1], &x, sizeof(x)) < sizeof(x)) {
        printf("write error\n");
        exit(1);
    }
}

int
read_int_or_zero(int *fds)
{
    int res;
    if (read(fds[0], &res, sizeof(int)) != sizeof(int)) {
        return 0;
    }
    return res;
}

void
run_in_fork(int *left_fds) {
    int right_fds[2] = {0, 0};
    open_pipe_or_exit(right_fds);

    int p = read_int_or_zero(left_fds);
    if (p == 0) {
        return;
    }

    if (fork_or_exit()) {
        close(left_fds[0]);
        close(right_fds[1]);
        wait(NULL);
        run_in_fork(right_fds);
    } else {
        printf("prime %d\n", p);
        for (int i; (i = read_int_or_zero(left_fds)); ) {
            if (i % p != 0) {
                write_int_or_exit(right_fds, i);
            }
        }
        close(left_fds[0]);
        close(right_fds[1]);
    }
}

int
main(int argc, char *argv[])
{
    int right_fds[2] = {0, 0};
    open_pipe_or_exit(right_fds);
    for (int i = 2; i < 35; ++i) {
        write_int_or_exit(right_fds, i);
    }
    close(right_fds[1]);

    run_in_fork(right_fds);
    exit(0);
}
