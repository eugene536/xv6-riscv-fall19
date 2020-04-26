#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *
shortname(char *expression)
{
    static char short_name[512];
    int expr_len = strlen(expression);
    if (expr_len >= sizeof(short_name)) {
        printf("too long: %s\n", expression);
        exit(1);
    }

    char *last_slash;
    for (last_slash = expression + expr_len; last_slash >= expression && *last_slash != '/'; --last_slash);

    return last_slash + 1;
}

void
find(char *starting_point, char *expression)
{
    int fd;
    if ((fd = open(starting_point, O_RDONLY)) < 0) {
        printf("open error\n");
        exit(1);
    }

    struct stat st;
    if(fstat(fd, &st) < 0){
        printf("find: cannot stat %s\n", starting_point);
        close(fd);
        exit(1);
    }

    switch(st.type){
        case T_FILE: {
            if (!strcmp(shortname(starting_point), expression)) {
                printf("%s\n", starting_point);
            }
        }
        break;

        case T_DIR: {
            char new_starting_point[512]; 
            if(strlen(starting_point) + strlen("/") + DIRSIZ + 1 > sizeof(new_starting_point)){
                printf("find: path too long\n");
                break;
            }
            strcpy(new_starting_point, starting_point);
            char *p = new_starting_point + strlen(starting_point);
            *p = '/';
            p++;
            struct dirent de;

            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                  continue;

                if (de.name[0] == '.') {
                    if (de.name[1] == '\0' || de.name[1] == '.') {
                        continue;
                    }
                }

                memmove(p, de.name, DIRSIZ);
                *(p + DIRSIZ) = '\0';

                find(new_starting_point, expression);
            }
        }
        break;
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc <= 2) {
    fprintf(2, "usage: find starting-point expression\n");
    exit(1);
  }

  char *starting_point = argv[1];
  char *expression = argv[2];

  find(starting_point, expression);

  exit(0);
}
