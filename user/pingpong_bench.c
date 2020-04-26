#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// #define uptime clock
// #include <stdio.h>
// #include <sys/wait.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <time.h>


// return double causes
// usertrap(): unexpected scause 0x0000000000000002 pid=3
//          sepc=0x0000000000000028 stval=0x0000000000000000
int
time_elapsed(int *start, int *end)
{
    if (start != NULL && end != NULL) {
        return *end - *start;
    }
    return 0;
}

void
open_pipe_or_exit(int *fds)
{
    if (pipe(fds) != 0) {
        printf("pipe open failure\n");
        exit(0);
    }
}

void
upd_time(int *end) {
    *end = uptime();
}

unsigned
ping_pong(int *read_fd, int *write_fd, int *start, int *end)
{
    close(read_fd[1]);
    close(write_fd[0]);

    upd_time(start);
    upd_time(end);

    char value;
    unsigned int cnt_iter;
    for (cnt_iter = 0; time_elapsed(start, end) < CLOCKS_PER_SEC; ++cnt_iter) {
        if (cnt_iter % 32 == 0) {
            upd_time(end);
        }
        if (write(write_fd[1], "a", 1) <= 0) {
            upd_time(end);
            break;
        }
        if (read(read_fd[0], &value, 1) == 0) {
            upd_time(end);
            break;
        }
    }

    close(write_fd[1]);
    close(read_fd[0]);

    return cnt_iter;
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

    unsigned cnt_iter;
    int start, end;
    if (child_pid) {
        cnt_iter = ping_pong(child_to_parent_fds, parent_to_child_fds, &start, &end);
    } else {
        cnt_iter = ping_pong(parent_to_child_fds, child_to_parent_fds, &start, &end);
    }

    if (child_pid) {
        wait(NULL);
        printf("cnt trips: %d; elapsed time: %d", cnt_iter, time_elapsed(&start, &end) / CLOCKS_PER_SEC);
    }

    exit(0);
}
