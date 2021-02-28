#ifndef MINISHELL_H
#define MINISHELL_H

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void unix_error(char *msg);

//文件IO
int Open(const char *path,int oflag,mode_t mode);
int Openat(int fd,const char *path,int oflag,mode_t mode);
ssize_t Read(int fd,void *buf,size_t nbytes);
ssize_t Write(int fd,const void *buf,size_t nbytes);
int Chdir(const char *pathname);
char *Getcwd(char *buf,size_t size);

//文件和目录
int Stat(const char *pathname,struct stat *buf);
int Fstatat(int fd,const char *pathname,struct stat *buf,int flag);
int Access(const char *pathname,int mode);
int Faccessat(int fd,const char *pathname,int mode,int flag);

DIR *Opendir(const char *pathname);
struct dirent *Readdir(DIR *dp);

//标准IO
char *Fgets(char *buf,int n,FILE *fp);

//系统文件信息
struct passwd *Getpwuid(uid_t uid);
struct group *Getgrgid(gid_t gid);

#endif
