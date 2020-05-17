#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"
#include <stdbool.h>

typedef struct {
    char *args[MAXARG + 1];
    char *in_name;
    char *out_name;
    int pipe_in;
    int pipe_out;
} process;

int
fork_or_exit(void)
{
    int child_pid = fork();
    if (child_pid < 0) {
        fprintf(2, "fork failure\n");
        exit(1);
    }
    return child_pid;
}

bool
is_space(char c)
{
    return c == ' ' || c == '\n';
}

bool
is_not_space(char c)
{
    return !is_space(c);
}

bool
is_special_symbol(char c)
{
    return c == '\0' || c == '|' || c == '<' || c == '>';
}

void
skip_if(char **args_line, bool (*pred)(char))
{
    for (; **args_line && pred(**args_line); (*args_line)++);
}

void
next_char(char **args_line)
{
    (*args_line)++;
    skip_if(args_line, is_space);
}

bool
separate_arg(char **args_line)
{
    skip_if(args_line, is_not_space);
    if (**args_line) {
        **args_line = '\0';
        next_char(args_line);
        return true;
    }
    return false;
}

void
get_redirection(char **args_line, char **redirect_name)
{
    next_char(args_line);
    *redirect_name = *args_line;
    separate_arg(args_line);
}

void
get_args(char **args_line, process *p)
{
    char **args = p->args;

    while (!is_special_symbol(**args_line)) {
        *args = *args_line;
        args++;

        if (!separate_arg(args_line)) {
            break;
        }
    }
    *args = NULL;

    for (int i = 0; i < 2; ++i) {
        if (**args_line == '<') {
            get_redirection(args_line, &p->in_name);
        }

        if (**args_line == '>') {
            get_redirection(args_line, &p->out_name);
        }
    }
}

void
redirect_std_in_out(process *p)
{
    if (p->in_name) {
        close(0);
        open(p->in_name, O_RDONLY);
    }

    if (p->out_name) {
        close(1);
        open(p->out_name, O_WRONLY|O_CREATE);
    }
}

void
run_with_pipe(process *lhs, process *rhs)
{
    int pipefd[2];
    pipe(pipefd);
    lhs->pipe_in = rhs->pipe_in = pipefd[0];
    lhs->pipe_out = rhs->pipe_out = pipefd[1];

    if (fork_or_exit()) {
        if (fork_or_exit()) {
            close(pipefd[0]);
            close(pipefd[1]);

            wait(NULL);
            wait(NULL);
        } else {
            close(rhs->pipe_out);
            close(0);
            dup(rhs->pipe_in);
            close(rhs->pipe_in);

            redirect_std_in_out(rhs);

            exec(rhs->args[0], rhs->args);
        }
    } else {
        close(lhs->pipe_in);
        close(1);
        dup(lhs->pipe_out);
        close(lhs->pipe_out);

        redirect_std_in_out(lhs);

        exec(lhs->args[0], lhs->args);
    }
}

void
run(process *p)
{
    if (fork_or_exit()) {
        wait(NULL);
    } else {
        redirect_std_in_out(p);
        exec(p->args[0], p->args);
    }
}

int
main(int argc, char *argv[])
{
    if(argc > 1){
      fprintf(2, "usage: nsh\n");
      exit(1);
    }

    char one_line[512];
    while (1) {
        printf("@ ");
        char *args_line = gets(one_line, sizeof(one_line));
        if (strlen(args_line) == 0) {
            break;
        }
        
        skip_if(&args_line, is_space);
        process lhs = {0};
        get_args(&args_line, &lhs);

        if (*args_line == '|') { 
            next_char(&args_line);
            process rhs = {0};
            get_args(&args_line, &rhs);

            run_with_pipe(&lhs, &rhs);
        } else {
            run(&lhs);
        }
    }
    exit(0);
}
