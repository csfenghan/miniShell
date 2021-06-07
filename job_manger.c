#include "job_manger.h"

static struct job_t *job_head = NULL, *job_tail = NULL;
static int next_jid = 0;

void init_job() {
	job_head = Malloc(sizeof(struct job_t));
	job_tail = Malloc(sizeof(struct job_t));
	job_head->prev = NULL;
	job_head->next = job_tail;
	job_tail->prev = job_head;
	job_tail->next = NULL;
}

struct job_t *create_job() {
	struct job_t *job;

	job = Malloc(sizeof(struct job_t));

	job->jid = -1;
	job->process_count = 0;
	job->done_count = 0;
	job->prev = NULL;
	job->next = NULL;

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

	job->prev = job_tail->prev;
	job->next = job_tail;
	job_tail->prev->next = job;
	job_tail->prev = job;
}

void list_job() {
	struct job_t *curr;

	curr = job_head->next;
	while (curr != job_tail) {
		printf("[%d]\n", curr->jid);
		struct process_info *info = curr->process_head;
		while (info != NULL) {
			char *state;
			switch (info->state) {
				case FORGROUND_RUNNING:
					state = "f_running";
					break;
				case BACKGROUND_RUNNING:
					state = "b_running";
					break;
				case STOP:
					state = "stop";
					break;
				case DONE:
					state = "done";
					break;
				case UNDEFINE:
				default:
					state = "undefine";
					break;
			}
			printf("    %s    %s\n", state, info->cmdline);
			info = info->next;
		}

		curr = curr->next;
	}
}

void destroy_job(struct job_t *job) {
	struct process_info *info;

	job->prev->next = job->next;
	job->next->prev = job->prev;

	// free the process_info list
	info = job->process_head;
	while (info != NULL) {
		struct process_info *temp = info;
		info = info->next;
		free(temp);
	}

	if (next_jid == job->jid)
		next_jid--;
	// free the job_t
	free(job);
}

// return jid if a job is deleted,else retrun 0
int del_process(pid_t pid) {
	struct job_t *job;
	struct process_info *info;
	int result;

	for (job = job_head->next; job != job_tail; job = job->next) {
		for (info = job->process_head; info != NULL; info = info->next) {
			if (info->pid == pid) {
				goto found;
			}
		}
	}
	unix_error("del process error,not found pid!");

found:
	result=0;
	info->state = DONE;
	if (++(job->done_count) == job->process_count) {
		result=job->jid;
		destroy_job(job);
	}
	return result;
}
int get_fg_job(){
	struct job_t *job;
	for(job=job_head->next;job!=job_tail;job=job->next){
		if(job->process_head->state==FORGROUND_RUNNING)
			return job->jid;
	}

	return -1;
}
