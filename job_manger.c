#include <unix_api.h>
#include "job_manger.h"


/* 作业控制函数start */
void init_jobs(struct job_t *job_list) {
        for (int i = 0; i < MAX_JOBS; i++) {
                job_list[i].pid = -1;
                job_list[i].jid = -1;
                job_list[i].state = UDF;
        }
        next_jid = 0;
}

void add_job(struct job_t *job_list, pid_t pid, int state, char *cmdline) {
        job_list[next_jid].pid = pid;
        job_list[next_jid].jid = next_jid + 1;
        job_list[next_jid].state = state;
        strcpy(job_list[next_jid].cmdline, cmdline);

        if (++next_jid >= MAX_JOBS)
                next_jid = 0;
}

void del_job(struct job_t *job_list, pid_t jid) {
        if (job_list[jid - 1].jid == jid) {
                job_list[jid - 1].pid = -1;
                job_list[jid - 1].jid = -1;
                job_list[jid - 1].state = UDF;

                if (next_jid == jid)
                        next_jid--;
        }
}

void list_jobs(struct job_t *job_list) {
        char *state;

        for (int i = 0; i < next_jid; i++) {
                if (job_list[i].pid != -1) {
                        switch (job_list[i].state) {
                        case UDF:
                                state = "Undefined";
                                break;
                        case F_R:
                                state = "F Running";
                                break;
                        case B_R:
                                state = "B Running";
                                break;
                        case B_S:
                                state = "B Stopped";
                                break;
                        default:
                                state = "State error";
                                break;
                        }
                        printf("[%d]  %s  %s", job_list[i].jid, state, job_list[i].cmdline);
                        fflush(stdout);
                }
        }
}

pid_t get_fg_job(struct job_t *job_list) {

        for (int i = 0; i < next_jid; i++)
                if (job_list[i].state == F_R)
                        return job_list[i].pid;
        return -1;
}

pid_t jid2pid(struct job_t *job_list, pid_t jid) { return job_list[jid].pid; }

pid_t pid2jid(struct job_t *job_list, pid_t pid) {

        for (int i = 0; i < next_jid; i++) {
                if (job_list[i].pid == pid)
                        return job_list[i].jid;
        }
        return -1;
}

struct job_t *jid2job(struct job_t *job_list, pid_t jid) {
        if (jid <= 0 || jid > MAX_JOBS)
                return NULL;
        return &job_list[jid - 1];
}

struct job_t *pid2job(struct job_t *job_list, pid_t pid) {
        pid_t jid;

        if ((jid = pid2jid(job_list, pid)) != -1)
                return &job_list[jid - 1];
        return NULL;
}

void do_fgbg(char *argv[]) {
        pid_t jid;
        sigset_t mask_all, mask_prev;
        struct job_t *job;

        if (argv[1] == NULL)
                jid = next_jid;
        else
                jid = atoi(argv[1]);

        if (((job = jid2job(jobs, jid)) == NULL) ||
            job->pid == -1) { /* 如果作业号无效，则报错返回 */
                fprintf(stderr, "fg: %d:no such job\n", jid);
                return;
        }

        /* 发送信号启动进程，前台等待进程 */
        sigfillset(&mask_all);
        sigprocmask(SIG_BLOCK, &mask_all, &mask_prev);
        kill(job->pid, SIGCONT); /* 最好是通过返回的SIGCHLD信号来判断是否运行，这样至少图省事*/
        if (strcmp(argv[0], "fg") == 0) {
                tcsetpgrp(STDIN_FILENO, job->pid);
                flags = 0; /* 等待前台作业 */
                job->state = F_R;
                while (!flags)
                        sigsuspend(&mask_prev);
                tcsetpgrp(STDIN_FILENO, getpid());
        } else if (strcmp(argv[0], "bg") == 0) {
                job->state = B_R;
        }
        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
}


