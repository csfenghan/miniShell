#ifndef MINI_SHELL_H
#define MINI_SHELL_H

#include <sys/types.h>
#include <unix_api.h>

/* 作业控制参数start */
#define UDF 0 /* 未定义 */
#define F_R 1 /* 前台运行 */
#define B_R 2 /* 后台运行 */
#define B_S 3 /* 后台停止 */

#define MAX_JOBS 1024
struct job_t {
        pid_t pid;
        pid_t jid;
        int state;
        char cmdline[MAXLINE];
};
int next_jid;
int flags;
extern char **environ;
struct job_t jobs[MAX_JOBS];

/* 信号处理函数start */
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigquit_handler(int sig);
void sigchld_handler(int sig);
/* 信号处理函数end */

/* 作业控制函数start */
void init_jobs(struct job_t *job_list);
void add_job(struct job_t *job_list, pid_t pid, int state, char *cmdline);
void del_job(struct job_t *job_list, pid_t jid);
void list_jobs(struct job_t *job_list);
void list_jobs(struct job_t *job_list);
pid_t get_fg_job(struct job_t *job_list);
pid_t jid2pid(struct job_t *job_list, pid_t jid);
pid_t pid2jid(struct job_t *job_list, pid_t pid);
struct job_t *jid2job(struct job_t *job_list, pid_t jid);
struct job_t *pid2job(struct job_t *job_list, pid_t pid);
void do_fgbg(char *argv[]) ;
/* 作业控制函数end */

//命令解析
int parse_line(char *buf, char **argv) ;
#endif
