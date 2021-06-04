#include "builtin_command.h"
#include "job_manger.h"
#include "parser_args.h"
#include "signal_handler.h"
#include "unix_api.h"

int pipfd[2];
void process_builtin_command(struct cmd *cmd) {
        sigset_t mask_all, mask_prev;
        int fd_in, fd_out;

        // block all signals when manipulate the global varible pipfd[2]
        sigfillset(&mask_all);
        sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);

        // save the descriptor standard output and standard input
        fd_in = dup(STDIN_FILENO);
        fd_out = dup(STDOUT_FILENO);

        // built-in command are not require fork a child process
        // set the input source according to the type of the previous cmd
        switch (cmd->prev_special_type) {
        case CMD_SPECIAL_AND:
        case CMD_SPECIAL_DEFAULT:
                // if the type is CMD_SPECIAL_AND or CMD_SPECIAL_DEFAULT,do nothing
                break;
        case CMD_SPECIAL_PIPE:
                // if the type is CMD_SPECIAL_PIPE,it means that pipfd[2] has been
                // initialized,	so it can be used directly
                dup2(pipfd[0], STDIN_FILENO);
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
                dup2(pipfd[1], STDOUT_FILENO);
                close(pipfd[1]);
                break;
        case CMD_SPECIAL_LEFT_REDIR:
        case CMD_SPECIAL_RIGHT_REDIR:
        default:
                break;
        }

        // exec the command after the configuration
        exec_builtin_command(cmd->argv[0], cmd->argc, cmd->argv);

        // restore standard input and output after execution
        dup2(fd_in, STDIN_FILENO);
        dup2(fd_out, STDOUT_FILENO);

        sigprocmask(SIG_BLOCK, &mask_prev, NULL);

        close(fd_in);
        close(fd_out);
}

// extern command and other command are require fork a new process
void process_extern_command(struct cmd *cmd) {
        sigset_t mask_all, mask_chld, mask_prev;
        pid_t pid;
        int backup_in_fd;

        sigfillset(&mask_all);
        sigemptyset(&mask_chld);
        sigaddset(&mask_chld, SIGCHLD);

        sigprocmask(SIG_BLOCK, &mask_chld, &mask_prev);

        // set the input source according the type of the current cmd
        switch (cmd->prev_special_type) {
        case CMD_SPECIAL_AND:
        case CMD_SPECIAL_DEFAULT:
                // if the type is CMD_SPECIAL_AND or CMD_SPECIAL_DEFAULT,do nothing
                break;
        case CMD_SPECIAL_PIPE:
                // if the type is CMD_SPECIAL_PIPE,it means that pipfd[2] has been
                // initialized,	so it can be used directly
                dup2(pipfd[0], STDIN_FILENO);
                backup_in_fd = dup(STDIN_FILENO);
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
                pipe(pipfd);
                break;
        case CMD_SPECIAL_LEFT_REDIR:
        case CMD_SPECIAL_RIGHT_REDIR:
        default:
                break;
        }

        // exec the command
        if ((pid = Fork()) > 0) {
                sigprocmask(SIG_SETMASK, &mask_prev, NULL);

                if (cmd->prev_special_type == CMD_SPECIAL_PIPE) {
                        dup2(backup_in_fd, STDIN_FILENO);
                        close(backup_in_fd);
                }

                if (cmd->next_special_type == CMD_SPECIAL_PIPE) {
                        dup2(pipfd[1], STDOUT_FILENO);
                }
                close(pipfd[0]);
                close(pipfd[1]);

                if (cmd->cmd_type == CMD_POSITION_EXEC) {
                        if (execv(cmd->argv[0], cmd->argv) < 0) {
                                fprintf(stderr, "execv error:%s\n", strerror(errno));
                                exit(0);
                        }
                } else if (cmd->cmd_type == CMD_POSITION_EXTERN) {
                        if (execvp(cmd->argv[0], cmd->argv) < 0) {
                                fprintf(stderr, "execvp error:%s\n", strerror(errno));
                                exit(0);
                        }
                }
        }
        close(pipfd[1]);
        if (backup_in_fd)
                close(backup_in_fd);

        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
}

// execute the command accordign to struct cmd_list
void exec_cmd(struct cmd_list *cmd_list) {
        struct cmd *cmd;

        // exec the command
        for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
                if (cmd->argc == CMD_POSITION_BUILTIN)
                        process_builtin_command(cmd);
                else
                        process_extern_command(cmd);
        }
}

/*void eval(char *cmdline) {
        char *argv[MAXARGS];
        char buf[MAXLINE];
        int bg;
        pid_t pid;
        sigset_t mask_all, mask_chld, prev_mask;

        strcpy(buf, cmdline);
        bg = parse_line(buf, argv);

        if (argv[0] == NULL) {
                return;
        }

        if (!is_buildin_command(argv)) {
                sigfillset(&mask_all);
                sigemptyset(&mask_chld);
                sigaddset(&mask_chld, SIGCHLD);

                sigprocmask(SIG_BLOCK, &mask_chld, &prev_mask);
                if ((pid = Fork()) == 0) {
                        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        if (setpgid(0, 0) < 0)
                                unix_error("setpgid error");

                        if (execvp(argv[0], argv) < 0) {
                                printf("%s: Command not found.\n", argv[0]);
                                exit(0);
                        }
                }

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
}*/

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
