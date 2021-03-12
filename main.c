#include "unix_api.h"

#define MAXARGS	64

extern char **environ;

//解析输入，以空格为分界，将其划分为数组格式。如果是后台命令则返回1，否则默认前台目录返回0
int parse_line(char *buf,char **argv)
{
	char *delim;
	int argc;
	int bg=0;		//是否是后台任务

	buf[strlen(buf)-1]=' ';
	while(*buf&&(*buf==' '))
		buf++;

	argc=0;
	while((delim=strchr(buf,' '))){
		argv[argc++]=buf;
		*delim='\0';
		buf=delim+1;
		while(*buf&&(*buf==' '))
			buf++;
	}
	argv[argc]=NULL;

	if(argc==0)
		return 1;

	if((bg=(*argv[argc-1]=='&'))!=0)	//是否是后台任务
		argv[--argc]=NULL;

	return bg;	
}


//如果是内部命令，执行并返回1，否则返回0
int is_buildin_command(char **argv)
{
	if(strcmp(argv[0],"cd")==0){

	}
	else if(strcmp(argv[0],"pwd")==0){

	}
	return 0;
}

void eval(char *cmdline)
{
	char *argv[MAXARGS];	
	char buf[MAXLINE];
	int bg;
	pid_t pid;

	//解析命令
	strcpy(buf,cmdline);
	bg=parse_line(buf,argv);
	if(argv[0]==NULL)
		return;

	//执行命令
	if(!is_buildin_command(argv)){
		if((pid=Fork())==0){
			if(execve(argv[0],argv,environ)<0){
				printf("%s: Command not found.\n",argv[0]);
				exit(0);
			}
		}

		//如果是前台任务，则shell等待
		if(!bg){
			int status;
			if(waitpid(pid,&status,0)<0)
				unix_error("waitfd: waitpid error");
		}else
			printf("%d %s",pid,cmdline);
	}
}

int main(int argc,char *argv[])
{
	char cmdline[MAXLINE];	

	while(1){
		printf("%s>",getcwd(NULL,0));
		Fgets(cmdline,MAXLINE,stdin);
		eval(cmdline);
	}
}
