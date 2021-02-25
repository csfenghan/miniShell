#include <stdio.h>

#define BUF_SIZE 256
#define MAX_ARGS 16

const char *name="miniShell%:";


int get_cmd(char *buf,size_t n)
{
	printf("%s",name);
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
	int argc=0;
	char last_char=' ';

	while(*buf){
		if(*buf!=' '){		//当前在处理非空格
			if(last_char==' '){
				cmd_lines[argc++]=buf;
			}
		}
		else{				//当前在处理空格
			if(last_char!=' ')
				*buf='\0';
		}
		last_char=(*buf=='\0')?(' '):*buf;
		buf++;
	}
	return argc;
}

int main(int argc,char **argv)
{
	char buf[BUF_SIZE];	
	char *cmd_lines[MAX_ARGS];

	while(1){
		if(get_cmd(buf,BUF_SIZE)>0){
			parse_cmd(buf,cmd_lines,MAX_ARGS);
			int n=parse_cmd(buf,cmd_lines,MAX_ARGS);
			printf("argc:%d\n",n);
			for(int i=0;i<n;i++){
				printf("%s\n",cmd_lines[i]);
			}
		}
		else
			fprintf(stderr,"get_cmd error");
	}
	
}
