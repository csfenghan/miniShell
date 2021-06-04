#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

/* 信号处理函数start */
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigquit_handler(int sig);
void sigchld_handler(int sig);
/* 信号处理函数end */

#endif
