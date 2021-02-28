#include "miniShell.h"

void unix_error(char *msg)
{
	fprintf(stderr,"%s: %s\n",msg,strerror(errno));
	exit(0);
}

int Open(const char *path,int oflag,mode_t mode)
{
	int fd;
	if((fd=open(path,oflag,mode))<0)
		unix_error("open error");
	return fd;
}

ssize_t Read(int fd,void *buf,size_t nbytes)
{
	ssize_t n;
	if((n=read(fd,buf,nbytes))<0)
		unix_error("read error");

	return n;
}

ssize_t Write(int fd,const void *buf,size_t nbytes)
{
	ssize_t n;
	if((n=write(fd,buf,nbytes))<0)
		unix_error("write error");
		
	return n;
}

int Stat(const char *pathname,struct stat *buf)
{
	int n;
	if((n=stat(pathname,buf))<0)
		unix_error("stat error");

	return n;
}

int Access(const char *pathname,int mode)
{
	int n;
	if((n=access(pathname,mode))<0)
		unix_error("access error");

	return n;
}

DIR *Opendir(const char *pathname)
{
	DIR *dp;
	if((dp=opendir(pathname))==NULL)
		unix_error("opendir error");
	return dp;
}

struct dirent *Readdir(DIR *dp)
{
	struct dirent *dirp;
	dirp=readdir(dp);
	return dirp;
}
