// shell的主体部分

#include "builtin_command.h"
#include "job_manger.h"
#include "parser_args.h"
#include "signal_handler.h"
#include "unix_api.h"

// execute the command accordign to struct cmd_list
void exec_cmd(struct cmd_list *cmd_list) {
        int fd_in, fd_out;
        int pipfd[2];
        pid_t pid;
        sigset_t mask_all, mask_chld, prev_mask;
        struct cmd *cmd;

        sigfillset(&mask_all);
        sigemptyset(&mask_chld);
        sigaddset(&mask_chld, SIGCHLD);

        // save the descriptor standard output and standard input
        fd_in = dup(STDIN_FILENO);
        fd_out = dup(STDOUT_FILENO);

        // exec the command
        for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
                // built-in command are not require fork a child process
                if (cmd->cmd_type == CMD_POSITION_BUILTIN) {
                        // set the input source according to the type of the previous cmd
                        switch (cmd->prev_special_type) {
                        case CMD_SPECIAL_AND:
                        case CMD_SPECIAL_DEFAULT:
                                // if the type is CMD_SPECIAL_AND or CMD_SPECIAL_DEFAULT,do nothing
                                break;
                        case CMD_SPECIAL_PIPE:
                                // if the type is CMD_SPECIAL_PIPE,it means that pipfd[2] has been
                                // initialized,	so it can be used directly
                                dup2(STDIN_FILENO, pipfd[0]);
                                close(pipfd[0]);
                        default:
                                break;
                        }

                        // set the ouput source according the type of the current cmd
                        switch (cmd->next_special_type) {
                        case CMD_SPECIAL_AND:
                        case CMD_SPECIAL_DEFAULT:
                                break;
                        case CMD_SPECIAL_PIPE:
                                // if the type is pipe,then initialize the pipfd[2] and redirect the
                                // output
                                pipe(pipfd);
                                dup2(STDOUT_FILENO, pipfd[1]);
                                close(pipfd[1]);
                                break;
                        case CMD_SPECIAL_LEFT_REDIR:
                        case CMD_SPECIAL_RIGHT_REDIR:
                        default:
                                break;
                        }

                        // exec the command after the configuration
                        exec_builtin_command(cmd->argv[0], cmd->argc, cmd->argv);
                }
                // extern command and other command are require fork a new process
                else {
                        sigprocmask(SIG_BLOCK, &mask_chld, &prev_mask);
                }
        }
        close(fd_in);
        close(fd_out);
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

int main(int argc, char *argv[]) {
        char cmdline[MAXLINE];
        struct cmd_list *cmd_list;

        init_jobs(jobs);
        setenv("PATH", "/home/fenghan/miniShell/bin", 1);

        // setting up signal processing functions
        Signal(SIGINT, sigint_handler);
        Signal(SIGTSTP, sigtstp_handler);
        Signal(SIGQUIT, sigquit_handler);
        Signal(SIGCHLD, sigchld_handler);

        while (1) {
                // waiting for user input
                printf("%s>", getcwd(NULL, 0));
                fflush(stdout);
                Fgets(cmdline, MAXLINE, stdin);
                if (feof(stdin))
                        exit(0);

                // parser the command
                cmd_list = create_cmd_list(cmdline);
                if (cmd_list != NULL) {
                        // execute the command
                        exec_cmd(cmd_list);
                        destroy_cmd_list(cmd_list);
                }
        }
}
