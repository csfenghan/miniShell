//shell的主体部分

#include "unix_api.h"
#include "mini_shell.h"


/* 如果是内部命令，执行并返回1，否则返回0 */
int is_buildin_command(char **argv) {
        if (strcmp(argv[0], "cd") == 0) { /* cd命令实现 */
                char *path;

                path = (argv[1] == NULL) ? getenv("HOME") : argv[1];
                if (argv[2] != NULL) {
                        fprintf(stderr, "cd: too many arguments\n");
                        return 1;
                }
                if (chdir(path) < 0)
                        fprintf(stderr, "chdir error\n");
                return 1;

        } else if (strcmp(argv[0], "pwd") == 0) { /* pwd命令实现 */
                printf("%s\n", getcwd(NULL, 0));
                return 1;

        } else if (strcmp(argv[0], "jobs") == 0) { /* jobs命令 */
                list_jobs(jobs);
                return 1;

        } else if (strcmp(argv[0], "fg") == 0) { /* fg命令 */
                do_fgbg(argv);
                return 1;

        } else if (strcmp(argv[0], "bg") == 0) {
                do_fgbg(argv);
                return 1;
        }

        return 0;
}

//根据cmdline执行对应的命令
void eval(char *cmdline) {
        char *argv[MAXARGS];
        char buf[MAXLINE];
        int bg;
        pid_t pid;
        sigset_t mask_all, mask_chld, prev_mask;

        //解析命令
        strcpy(buf, cmdline);
        bg = parse_line(buf, argv);

        if (argv[0] == NULL) {
                return;
        }

        // 执行命令 
        if (!is_buildin_command(argv)) {
                sigfillset(&mask_all);
                sigemptyset(&mask_chld);
                sigaddset(&mask_chld, SIGCHLD);

                sigprocmask(SIG_BLOCK, &mask_chld, &prev_mask);
                if ((pid = Fork()) == 0) {
                        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        if (setpgid(0, 0) < 0)
                                unix_error("setpgid error");

			//如果argv[0]以./开头，则是指定目录位置；否则在bin目录下搜索可执行文件
                        if (execvp(argv[0], argv) < 0) {
                                printf("%s: Command not found.\n", argv[0]);
                                exit(0);
                        }
                }

                /* 如果是前台任务，则shell等待 */
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                if (!bg) {
                        tcsetpgrp(STDIN_FILENO, pid);
                        add_job(jobs, pid, F_R, cmdline);
                        flags = 0;
                        while (!flags)
                                sigsuspend(&prev_mask);
                        tcsetpgrp(STDIN_FILENO, getpid());

                } else {
                        add_job(jobs, pid, B_R, cmdline);
                        printf("[%d]  %d %s\n", pid2jid(jobs, pid), pid, cmdline);
                }
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        }
}
/* Shell基本实现end */

/* main start */
int main(int argc, char *argv[]) {
        char cmdline[MAXLINE];

        /* 初始化作业控制*/
        init_jobs(jobs);
        setenv("PATH", "/home/fenghan/miniShell/bin", 1);

        /* 设置信号处理函数 */
        Signal(SIGINT, sigint_handler);
        Signal(SIGTSTP, sigtstp_handler);
        Signal(SIGQUIT, sigquit_handler);
        Signal(SIGCHLD, sigchld_handler);

        while (1) {
                printf("%s>", getcwd(NULL, 0));
                fflush(stdout);
                Fgets(cmdline, MAXLINE, stdin);
                if (feof(stdin))
                        exit(0);
                eval(cmdline);
        }
}
