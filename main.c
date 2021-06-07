#include "builtin_command.h"
#include "job_manger.h"
#include "parser_args.h"
#include "signal_handler.h"
#include "unix_api.h"

// execute the command accordign to struct cmd_list
void exec_cmd(struct cmd_list *cmd_list, char *cmdline) {
	pid_t pid;
	sigset_t mask_all, mask_prev,mask_chld;
	struct cmd *cmd;
	struct job_t *job;
	enum process_state state;
	int fd_in, fd_out;
	int pipfd[2] = {-1, -1};
	extern int forground_jid;

	job = create_job();
	state=(cmd_list->job_type==CMD_JOB_FG)?FORGROUND_RUNNING:BACKGROUND_RUNNING;

	// 0. save the standard input and output
	fd_in = dup(STDIN_FILENO);
	fd_out = dup(STDOUT_FILENO);

	// 1. block the chld signal
	sigemptyset(&mask_chld);
	sigaddset(&mask_chld,SIGCHLD);
	sigprocmask(SIG_BLOCK,&mask_chld,&mask_prev);

	for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
		// 1. set the input source according the type of the current cmd
		switch (cmd->prev_special_type) {
			case CMD_SPECIAL_AND:
				break;
			case CMD_SPECIAL_DEFAULT:
				break;
			case CMD_SPECIAL_PIPE:
				dup2(pipfd[0], STDIN_FILENO);
				close(pipfd[0]);
				pipfd[0] = -1;
			default:
				break;
		}

		// 2. set the ouput source according the type of the current cmd
		switch (cmd->next_special_type) {
			case CMD_SPECIAL_AND:
				break;
			case CMD_SPECIAL_DEFAULT:
				break;
			case CMD_SPECIAL_PIPE:
				if (pipfd[0] > 0 || pipfd[1] > 0){
					unix_error("pidfd[0] should be zero");
				}
				pipe(pipfd);
				dup2(pipfd[1], STDOUT_FILENO);

				close(pipfd[1]);
				pipfd[1] = -1;
				break;
			case CMD_SPECIAL_LEFT_REDIR:
				break;
			case CMD_SPECIAL_RIGHT_REDIR:
				break;
			default:
				break;
		}

		// 3. exec the command
		if (cmd->cmd_type == CMD_POSITION_BUILTIN)
			exec_builtin_command(cmd->argv[0], cmd->argc, cmd->argv);
		else {
			if ((pid = Fork()) == 0) {
				sigprocmask(SIG_SETMASK, &mask_prev, NULL);

				if (cmd->cmd_type == CMD_POSITION_EXEC)
					execv(cmd->argv[0], cmd->argv);
				else if (cmd->cmd_type == CMD_POSITION_EXTERN)
					execvp(cmd->argv[0], cmd->argv);

				unix_error("execvp extern command error");
			}
			add_process(job, pid, cmd->argv[0], state);
		}

		// 4. restore the standard input and output
		dup2(fd_in, STDIN_FILENO);
		dup2(fd_out, STDOUT_FILENO);
	}
	close(fd_in);
	close(fd_out);

	// 5. wait for forground job or destroy struct job depending on the cmd type
	if(job->process_head!=NULL){
		sigfillset(&mask_all);
		sigprocmask(SIG_BLOCK, &mask_all, NULL);

		add_job(job, cmdline);
		// waiting for forground job
		forground_jid=get_fg_job();
		while (forground_jid>0)
			sigsuspend(&mask_prev);

	}else{
		destroy_job(job);
	}
	sigprocmask(SIG_SETMASK, &mask_prev, NULL);
}
int main(int argc, char *argv[]) {
	char cmdline[MAXLINE];
	struct cmd_list *cmd_list;

	init_job();
	//setenv("PATH", "/home/fenghan/miniShell/bin", 1);

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
