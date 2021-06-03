#include <unix_api.h>
#include "mini_shell.h"

/* 解析输入，以空格为分界，将其划分为数组格式。如果是后台命令则返回1，否则默认前台目录返回0
 */
int parse_line(char *buf, char **argv) {
        char *delim;
        int argc;
        int bg = 0; //是否是后台任务

        buf[strlen(buf) - 1] = ' ';
        while (*buf && (*buf == ' '))
                buf++;

        argc = 0;
        while ((delim = strchr(buf, ' '))) {
                argv[argc++] = buf;
                *delim = '\0';
                buf = delim + 1;
                while (*buf && (*buf == ' '))
                        buf++;
        }
        argv[argc] = NULL;

        if (argc == 0)
                return 1;

        if ((bg = (*argv[argc - 1] == '&')) != 0) //是否是后台任务
                argv[--argc] = NULL;

        return bg;
}


