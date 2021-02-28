#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUF_SIZE 		256			//最大命令行输入字符数量
#define MAX_ARGS 		16			//最大参数数量
#define MAX_PATH_LEN	64			//最长路径限制

char *PATH="/home/fenghan/miniShell/user/bin";
char *env[]={"PATH=/home/fenghan/miniShell/user/bin"};
char curr_path[MAX_PATH_LEN];		//当前路径
char *name="fh%";					//提示符

//不声明的话会有warning，很奇怪
int faccessat(int fd,char *pathname,int mode,int flag);
int openat(int __fd,char *__file,int __oflag,...);
int fexecve(int __fd,char  *const *__argv,char *const *__envp);

/*
 *功能：变量初始化
 * */
void init()
{
	//获取当前路径
	if(getcwd(curr_path,MAX_PATH_LEN)==NULL){
		perror("getcwd error");
	}

}

/*
 *功能：显示提示符，捕获输入字符
 *成功返回1，失败返回-1	
 * */
int get_cmd(char *buf,size_t n)
{
	printf("%s",name);
	printf("%s",curr_path);
	printf("%%: ");

	if(fgets(buf,n,stdin)==NULL){
		perror("fgets error");
		return -1;
	}
	return 1;
}

/*
 * 功能：解析字符串
 *输入：
 *	buf：要解析的字符串
 *	cmd_lines:保存结果
 *	n:cmd_lines的长度
 *返回：
 *	arg的数量
 * */
int parse_cmd(char *buf,char **cmd_lines,int n)
{
	int args=0;
	char last_char=' ';

	while(*buf!='\n'){
		if(*buf!=' '){		//当前在处理非空格
			if(last_char==' '){
				cmd_lines[args++]=buf;
			}
		}
		else{				//当前在处理空格
			if(last_char!=' ')
				*buf='\0';
		}
		last_char=(*buf=='\0')?(' '):*buf;
		buf++;
	}
	*buf='\0';
	cmd_lines[args]=NULL;

	return args;
}

//根据解析后的输入内容cmd_lines运行命令
void run_cmd(char **cmd_lines,int n)
{
	//cd命令
	if(strcmp(cmd_lines[0],"cd")==0){
		if(n!=2){
			fprintf(stderr,"cd : too many arguments\n");
			return;
		}
		if(chdir(cmd_lines[1])<0){
			perror("chdir error");
			return;
		}
		if(getcwd(curr_path,MAX_PATH_LEN)==NULL){
			perror("getcwd error after chdir");
			return;
		}
	}	
	//pwd命令
	else if(strcmp(cmd_lines[0],"pwd")==0){
		char buf[MAX_PATH_LEN];
		if(getcwd(buf,MAX_PATH_LEN)==NULL){
			perror("getcwd error");
			return;
		}
		printf("%s\n",buf);
	}
	//执行bin目录中的命令，或者执行其他可执行文件
	else{
		//根据路径执行命令
		if(cmd_lines[0][0]=='.'&&cmd_lines[0][1]=='/'){
			if(access(cmd_lines[0],F_OK)<0){
				perror("cmd not exist");
				return;
			}
			if(fork()==0){
				execv(cmd_lines[0],cmd_lines);
				perror("execve error");
				return;
			}
			wait(NULL);
		}
		//在usr/bin中查找命令
		else{
			int fd1,fd2;
			if((fd1=open(PATH,O_RDONLY))<0){
				perror("open PATH error");
				return;
			}
				
			if(faccessat(fd1,cmd_lines[0],F_OK,0)<0){
				perror("cmd not exist");
				return;
			}
			fd2=openat(fd1,cmd_lines[0],O_RDONLY);
			if(fork()==0){
				fexecve(fd2,cmd_lines,env);
				perror("execve error");
				return;
			}
			wait(NULL);
		}

	}
}


int main(int argc,char **argv)
{
	char buf[BUF_SIZE];	
	char *cmd_lines[MAX_ARGS]={};

	init();

	while(1){
		if(get_cmd(buf,BUF_SIZE)>0){
			int args=parse_cmd(buf,cmd_lines,MAX_ARGS);
			run_cmd(cmd_lines,args);
		}
		else
			fprintf(stderr,"get_cmd error");
	}
	
}
