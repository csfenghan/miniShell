#include "job_manger.h"
#include <unix_api.h>

int forground_jid= 0;

/* 信号处理函数start */
void sigint_handler(int sig) {
	/*        int old_errno;
		  pid_t pid;

		  old_errno = errno;
		  pid = get_fg_job(jobs);
		  if (pid != -1) {
		  kill(pid, sig);
		  }
		  errno = old_errno;
		  */
}

void sigtstp_handler(int sig) {
	/*int old_errno;
	  pid_t pid;

	  old_errno = errno;
	  pid = get_fg_job(jobs);
	  if (pid != -1) {
	  kill(pid, sig);
	  }
	  errno = old_errno;
	  return;
	  */
}

void sigquit_handler(int sig) {
	/*        int old_errno;
		  pid_t pid;

		  old_errno = errno;
		  pid = get_fg_job(jobs);
		  if (pid != -1) {
		  kill(pid, sig);
		  }
		  errno = old_errno;
		  return;
		  */
}

void sigchld_handler(int sig) {
	pid_t pid;
	int status, old_errno;
	sigset_t mask_all, mask_prev;

	old_errno = errno;
	sigfillset(&mask_all);
	sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);

	forground_jid= get_fg_job();
	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED))>0){
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			if (del_process(pid) == forground_jid)
				forground_jid= 0;
			printf("pid:%d normally exit\n", pid);

		} else if (WIFSTOPPED(status)) { // stop
			// if (fg_pid == pid)
			//       flags = 1;
			// job = pid2job(jobs, pid);
			// job->state = B_S;

		} else if (WIFCONTINUED(status)) {
		}
	}

	sigprocmask(SIG_SETMASK, &mask_prev, NULL);
	errno = old_errno;
}
