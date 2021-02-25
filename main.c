#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BIN_PATH	(./bin)		//执行文件搜索路径
#define BUF_SIZE	1024		//输入缓存大小
#define MAX_CMD_NUM	64			//最大支持的命令数
#define MAX_CMD_LEN	64			//单个命令的最大长度

void parser_cmd(char *,char **);
void run_cmd(char **);

int main(int argc,char **argv)
{
	char input_buf[BUF_SIZE];		//输入缓存
	char *cmd_args[MAX_CMD_NUM];	//解析后的输入

	//给cmd_args预先分配好内存，用于存放解析后的命令
	for(int i=0;i<MAX_CMD_NUM;i++){
		cmd_args[i]=(char *)malloc(MAX_CMD_LEN);
		memset(cmd_args[i],0,MAX_CMD_LEN);
	}	

	while(1){
		printf("miniShell%%:");
		if(fgets(input_buf,BUF_SIZE,stdin)==NULL){
			perror("fgets error");
		}
		parser_cmd(input_buf,cmd_args);
		run_cmd(cmd_args);
	}
}

////////////////////////////////////////////////////////////////
void parser_cmd(char *input,char** output)
{
	char *ptr=input,last_char=' ';
	int i=0,j=0;

	while(*ptr){
		//遇到换行符则返回
		if(*ptr=='\n'){
			output[i++][j++]=0;
			output[i][0]=0;
			return;
		}

		else if(*ptr!=' '){
			output[i][j++]=*ptr;
		}

		//如果在遇到命令后第一次遇到空格，需要将最后一位置为0
		else if(last_char!=' '){
			output[i++][j]=0;
			j=0;
		}
		last_char=*ptr++;
	}
}

void run_cmd(char **cmd_args)
{
	if(strcmp(cmd_args[0],"cd")==0){
		if(cmd_args[2][0]!=0){
			fprintf(stderr,"minibash: cd : too many arguments\n");
			return;
		}
		if(chdir(cmd_args[1])<0){
			perror("cd error");
			return;
		}	
	}
	else if(strcmp(cmd_args[0],"pwd")==0){
		if(cmd_args[1][0]!=0){
			fprintf(stderr,"minibash: pwd : too many arguments\n");
			return;
		}
		char buf[BUF_SIZE];
		if(getcwd(buf,BUF_SIZE)==NULL){
			perror("pwd error");
			return;
		}
		printf("%s\n",buf);
	}
}
