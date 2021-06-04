#ifndef MINI_SHELL_H
#define MINI_SHELL_H

#include <sys/types.h>
#include <unix_api.h>

enum job_state {
        UDF = 0,
        F_R = 1, // forground running
        B_R = 2, // background running
        B_S = 3, // background stop
};

#define MAX_JOBS 1024
struct job_t {
        pid_t pid;
        pid_t jid;
        enum job_state state;
        char cmdline[MAXLINE];
};
int flags;
struct job_t jobs[MAX_JOBS];

void init_jobs(struct job_t *job_list);
void add_job(struct job_t *job_list, pid_t pid, int state, char *cmdline);
void del_job(struct job_t *job_list, pid_t jid);
void list_jobs(struct job_t *job_list);
pid_t get_fg_job(struct job_t *job_list);
pid_t jid2pid(struct job_t *job_list, pid_t jid);
pid_t pid2jid(struct job_t *job_list, pid_t pid);
struct job_t *jid2job(struct job_t *job_list, pid_t jid);
struct job_t *pid2job(struct job_t *job_list, pid_t pid);
void do_fgbg(char *argv[]);

#endif
