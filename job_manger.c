#include "job_manger.h"

static struct job_t *job_list = NULL, *curr_job = NULL;
static int next_jid = 0;

struct job_t *create_job() {
        struct job_t *job;

        job = Malloc(sizeof(struct job_t));

        job->jid = -1;
        job->process_count = 0;
        job->done_count = 0;

        return job;
}

void add_process(struct job_t *job, pid_t pid, char *cmdline, enum process_state state) {
        if (job->process_head == NULL) {
                job->process_head = Malloc(sizeof(struct process_info));
                job->process_tail = job->process_head;
        } else {
                job->process_tail->next =
                    (struct process_info *)Malloc(sizeof(struct process_info));
                job->process_tail = job->process_tail->next;
        }

        job->process_tail->pid = pid;
        job->process_tail->state = state;
        job->process_tail->next = NULL;
        strcpy(job->process_tail->cmdline, cmdline);

        job->process_count++;
}
void add_job(struct job_t *job, char *cmdline) {
        job->jid = ++next_jid;

        if (job_list == NULL) {
                job_list = job;
                curr_job = job;
        } else {
                curr_job->next = job;
                curr_job = curr_job->next;
        }
}

void list_job() {
        struct job_t *curr;

        curr = job_list;
        while (curr != NULL) {
                printf("[%d]", curr->jid);
                struct process_info *info = curr->process_head;
                while (info != NULL) {
                        printf(" running %s\n", info->cmdline);
                        info = info->next;
                }

                curr = curr->next;
        }
}
