#include "job_manger.h"
#include <unix_api.h>

int is_forground_running = 1;

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
        pid_t pid, fg_pid;
        int status, old_errno;
        sigset_t mask_all, mask_prev;

        old_errno = errno;
        sigfillset(&mask_all);
        sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);

        // fg_pid = get_fg_job(jobs);
        pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
                if (del_process(pid) == 1)
                        is_forground_running = 0;

        } else if (WIFSTOPPED(status)) { // stop
                // if (fg_pid == pid)
                //       flags = 1;
                // job = pid2job(jobs, pid);
                // job->state = B_S;

        } else if (WIFCONTINUED(status)) {
        }

        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
        errno = old_errno;
}
