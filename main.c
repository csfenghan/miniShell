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
                break;
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

        sigprocmask(SIG_SETMASK, &mask_prev, NULL);

        close(fd_in);
        close(fd_out);
}

// extern command and other command are require fork a new process
pid_t process_extern_command(struct cmd *cmd) {
        sigset_t mask_all, mask_prev;
        pid_t pid;
        int backup_in_fd;

        sigfillset(&mask_all);
        sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);

        // set the input source according the type of the current cmd
        switch (cmd->prev_special_type) {
        case CMD_SPECIAL_AND:
        case CMD_SPECIAL_DEFAULT:
                // if the type is CMD_SPECIAL_AND or CMD_SPECIAL_DEFAULT,do nothing
                break;
        case CMD_SPECIAL_PIPE:
                // if the type is CMD_SPECIAL_PIPE,it means that pipfd[2] has been
                // initialized,	so it can be used directly
                backup_in_fd = dup(pipfd[0]);
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
        if ((pid = Fork()) == 0) {
                sigprocmask(SIG_SETMASK, &mask_prev, NULL);

                if (cmd->prev_special_type == CMD_SPECIAL_PIPE) {
                        dup2(backup_in_fd, STDIN_FILENO);
                        close(backup_in_fd);
                }

                if (cmd->next_special_type == CMD_SPECIAL_PIPE) {
                        dup2(pipfd[1], STDOUT_FILENO);
                }

                /*printf("printf the output,%d\n", cmd->argc);
                for (int i = 0; cmd->argv[i] != (char *)0; i++)
                        printf("args%d:%s\n", i, cmd->argv[i]);
                fflush(stdout);*/

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
                } else {
                        fprintf(stderr, "unknow command type\n");
                }
        }
        sigprocmask(SIG_SETMASK, &mask_prev, NULL);

        return pid;
}

// execute the command accordign to struct cmd_list
void exec_cmd(struct cmd_list *cmd_list, char *cmdline) {
        extern int is_forground_running;
        pid_t pid;
        sigset_t mask_all, mask_prev;
        struct cmd *cmd;
        struct job_t *job;
        enum process_state state;
        int is_contain_extern_command;

        job = create_job();
        if (cmd_list->job_type == CMD_JOB_FG) {
                is_forground_running = 1;
                state = BACKGROUND_RUNNING;
        } else {
                is_forground_running = 0;
                state = FORGROUND_RUNNING;
        }

        // exec the command
        is_contain_extern_command = 0;
        for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
                if (cmd->cmd_type == CMD_POSITION_BUILTIN)
                        process_builtin_command(cmd);
                else {
                        pid = process_extern_command(cmd);
                        add_process(job, pid, cmd->argv[0], state);
                        is_contain_extern_command = 1;
                }
        }
        if (is_contain_extern_command) {
                sigfillset(&mask_all);
                sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);
                add_job(job, cmdline);
		fflush(stdout);
                while (is_forground_running)
                        sigsuspend(&mask_prev);

                sigprocmask(SIG_SETMASK, &mask_prev, NULL);
        }
}
int main(int argc, char *argv[]) {
        char cmdline[MAXLINE];
        struct cmd_list *cmd_list;

        init_job();
        setenv("PATH", "/home/fenghan/miniShell/bin", 1);

        // setting up signal processing functions
        // Signal(SIGINT, sigint_handler);
        // Signal(SIGTSTP, sigtstp_handler);
        // Signal(SIGQUIT, sigquit_handler);
        Signal(SIGCHLD, sigchld_handler);

        while (1) {
                // waiting for user input
                printf("%s>", getcwd(NULL, 0));
                fflush(stdout);
                if (fgets(cmdline, MAXLINE, stdin) == NULL) {
                        perror("fgets error");
                        exit(0);
                }
                if (feof(stdin))
                        exit(0);

                // parser the command
                cmd_list = create_cmd_list(cmdline);
                if (cmd_list != NULL) {
                        // execute the command
                        exec_cmd(cmd_list, cmdline);
                        destroy_cmd_list(cmd_list);
                }
        }
}
