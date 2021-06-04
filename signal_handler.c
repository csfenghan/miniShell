#include <unix_api.h>
#include "job_manger.h"

/* 信号处理函数start */
void sigint_handler(int sig) {
        int old_errno;
        pid_t pid;

        old_errno = errno;
        pid = get_fg_job(jobs);
        if (pid != -1) {
                kill(pid, sig);
        }
        errno = old_errno;
}

void sigtstp_handler(int sig) {
        int old_errno;
        pid_t pid;

        old_errno = errno;
        pid = get_fg_job(jobs);
        if (pid != -1) {
                kill(pid, sig);
        }
        errno = old_errno;
        return;
}

void sigquit_handler(int sig) {
        int old_errno;
        pid_t pid;

        old_errno = errno;
        pid = get_fg_job(jobs);
        if (pid != -1) {
                kill(pid, sig);
        }
        errno = old_errno;
        return;
}

void sigchld_handler(int sig) {
        pid_t pid, fg_pid;
        struct job_t *job;
        int status, old_errno;
        sigset_t mask_all, mask_prev;

        old_errno = errno;
        sigfillset(&mask_all);
        sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);

        fg_pid = get_fg_job(jobs);
        pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (WIFEXITED(status)) { /*正常退出*/
                if (fg_pid == pid)
                        flags = 1;
                del_job(jobs, pid2jid(jobs, pid));

        } else if (WIFSIGNALED(status)) { /*被信号杀死*/
                if (fg_pid == pid)
                        flags = 1;
                del_job(jobs, pid2jid(jobs, pid));

        } else if (WIFSTOPPED(status)) { /*进程停止*/
                if (fg_pid == pid)
                        flags = 1;
                job = pid2job(jobs, pid);
                job->state = B_S;

        } else if (WIFCONTINUED(status)) { /* 接受并打印continue信息，kill发送者负责更改jobs */
        }

        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
        errno = old_errno;
}
/* 信号处理函数end */


