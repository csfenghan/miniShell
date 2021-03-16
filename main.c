#include "unix_api.h"

/* 全局变量start */

extern char **environ;
int flags;

/* 全局变量end */

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
struct job_t jobs[MAX_JOBS];
int next_jid;
/* 作业控制参数end */

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


/* 作业控制函数end */

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

void sigquit_handler(int sig){
	int old_errno;
	pid_t pid;

	old_errno=errno;
	pid=get_fg_job(jobs);
	if(pid!=-1){
		kill(pid,sig);
	}
	errno=old_errno;
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

        } else if (WIFCONTINUED(
                       status)) { /* 只是接受并打印continue信息，jobs的改变由kill发送者更改 */
        }

        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
        errno = old_errno;
}
/* 信号处理函数end */

/* Shell主体部分start */
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
		tcsetpgrp(STDIN_FILENO,job->pid);
                flags = 0; /* 等待前台作业 */
                job->state = F_R;
                while (!flags)
                        sigsuspend(&mask_prev);
		tcsetpgrp(STDIN_FILENO,getpid());
        } else if (strcmp(argv[0], "bg") == 0) {
                job->state = B_R;
        }
        sigprocmask(SIG_SETMASK, &mask_prev, NULL);
}

/* 解析输入，以空格为分界，将其划分为数组格式。如果是后台命令则返回1，否则默认前台目录返回0
 */
int parse_line(char *buf, char **argv) {
        char *delim;
        int argc;
        int bg = 0; //是否是后台任务

        buf[strlen(buf) - 1] = ' ';
        while (*buf && (*buf == ' '))
                buf++;

        argc = 0;
        while ((delim = strchr(buf, ' '))) {
                argv[argc++] = buf;
                *delim = '\0';
                buf = delim + 1;
                while (*buf && (*buf == ' '))
                        buf++;
        }
        argv[argc] = NULL;

        if (argc == 0)
                return 1;

        if ((bg = (*argv[argc - 1] == '&')) != 0) //是否是后台任务
                argv[--argc] = NULL;

        return bg;
}

/* 如果是内部命令，执行并返回1，否则返回0 */
int is_buildin_command(char **argv) {
        if (strcmp(argv[0], "cd") == 0) { /* cd命令实现 */
                char *path;

                path = (argv[1] == NULL) ? getenv("HOME") : argv[1];
                if (argv[2] != NULL) {
                        fprintf(stderr, "cd: too many arguments\n");
                        return 1;
                }
                if (chdir(path) < 0)
                        fprintf(stderr, "chdir error\n");
                return 1;

        } else if (strcmp(argv[0], "pwd") == 0) { /* pwd命令实现 */
                printf("%s\n", getcwd(NULL, 0));
                return 1;

        } else if (strcmp(argv[0], "jobs") == 0) { /* jobs命令 */
                list_jobs(jobs);
                return 1;

        } else if (strcmp(argv[0], "fg") == 0) { /* fg命令 */
                do_fgbg(argv);
                return 1;

        } else if (strcmp(argv[0], "bg") == 0) {
                do_fgbg(argv);
                return 1;
        }

        return 0;
}

void eval(char *cmdline) {
        char *argv[MAXARGS];
        char buf[MAXLINE];
        int bg;
        pid_t pid;
        sigset_t mask_all, mask_chld, prev_mask;

        //解析命令
        strcpy(buf, cmdline);
        bg = parse_line(buf, argv);

        if (argv[0] == NULL) {
                return;
        }

        /* 执行命令 */
        if (!is_buildin_command(argv)) {
                sigfillset(&mask_all);
                sigemptyset(&mask_chld);
                sigaddset(&mask_chld, SIGCHLD);

                sigprocmask(SIG_BLOCK, &mask_chld, &prev_mask);
                if ((pid = Fork()) == 0) {
                        sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        if (setpgid(0, 0) < 0)
                                unix_error("setpgid error");
                        if (execvp(argv[0], argv) < 0) {
                                printf("%s: Command not found.\n", argv[0]);
                                exit(0);
                        }
                }

                /* 如果是前台任务，则shell等待 */
                sigprocmask(SIG_BLOCK, &mask_all, NULL);
                if (!bg) {
			tcsetpgrp(STDIN_FILENO,pid);
                        add_job(jobs, pid, F_R, cmdline);
                        flags = 0;
                        while (!flags)
                                sigsuspend(&prev_mask);
			tcsetpgrp(STDIN_FILENO,getpid());

                } else {
                        add_job(jobs, pid, B_R, cmdline);
                        printf("[%d]  %d %s\n", pid2jid(jobs, pid), pid, cmdline);
                }
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
        }
}
/* Shell基本实现end */

/* main start */
int main(int argc, char *argv[]) {
        char cmdline[MAXLINE];

        /* 初始化作业控制*/
        init_jobs(jobs);
	setenv("PATH","/home/fenghan/miniShell/bin",1);

        /* 设置信号处理函数 */
        Signal(SIGINT, sigint_handler);
        Signal(SIGTSTP, sigtstp_handler);
	Signal(SIGQUIT,sigquit_handler);
        Signal(SIGCHLD, sigchld_handler);

        while (1) {
                printf("%s>", getcwd(NULL, 0));
		fflush(stdout);
                Fgets(cmdline, MAXLINE, stdin);
		if(feof(stdin))
			exit(0);
                eval(cmdline);
        }
}
/* main end */
