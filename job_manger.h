#ifndef JOB_MANGER_H
#define JOB_MANGER_H

#include "unix_api.h"

enum process_state {
        UNDEFINE = 0,
        FORGROUND_RUNNING = 1,
        BACKGROUND_RUNNING = 2,
        STOP = 3,
        DONE = 4,
};

struct process_info {
        pid_t pid;
        char cmdline[MAXLINE];
        enum process_state state;
        struct process_info *next;
};

struct job_t {
        int jid;
        int process_count;
        int done_count;
        struct process_info *process_head;
        struct process_info *process_tail;
        char cmdline[MAXLINE];
	struct job_t *next;
};

struct job_t *create_job();
void add_process(struct job_t *job,pid_t pid,char *cmdline,enum process_state state);
void add_job(struct job_t *job,char *cmdline);
void list_job();

void del_process();

#endif
