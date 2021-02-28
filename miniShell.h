#ifndef MINISHELL_H
#define MINISHELL_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void unix_error(char *msg);

//文件IO
int Open(const char *path,int oflag,mode_t mode);
ssize_t Read(int fd,void *buf,size_t nbytes);
ssize_t Write(int fd,const void *buf,size_t nbytes);

//文件和目录
int Stat(const char *pathname,struct stat *buf);
int Access(const char *pathname,int mode);

DIR *Opendir(const char *pathname);
struct dirent *Readdir(DIR *dp);

#endif
