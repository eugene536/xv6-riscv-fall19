#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc <= 1){
    fprintf(2, "usage: sleep seconds\n");
    exit(1);
  }

  int cnt_seconds = atoi(argv[1]);
  sleep(cnt_seconds * CLOCKS_PER_SEC);
  exit(0);
}

