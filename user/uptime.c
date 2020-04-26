#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int sum_seconds = uptime() / CLOCKS_PER_SEC;
    int cnt_hours = sum_seconds / 3600;
    int cnt_minutes = sum_seconds / 60 - cnt_hours * 60;
    int cnt_seconds = sum_seconds % 60;
    printf("up %d:%d:%d h:m:s \n", cnt_hours, cnt_minutes, cnt_seconds);
    exit(0);
}
