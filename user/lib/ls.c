#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include<string.h>
#include <sys/stat.h>

#define MAX_PATH_LEN 64

//////////////////////////////////

#define LS_A	(1<<0)
#define LS_H	(1<<8)
#define LS_I	(1<<9)
#define LS_L	(1<<12)

#define IS_SET_A(x)	((x)&LS_A)
#define IS_SET_H(x)	((x)&LS_H)
#define IS_SET_I(x)	((x)&LS_I)
#define IS_SET_L(x)	((x)&LS_L)


//给符号添加指定的flags
int __parse_flags(char,int*);
inline int __parse_flags(char c,int *flags)
{
	switch(c){
		case 'a':
			*flags|=LS_A;
			break;
		case 'h':
			*flags|=LS_H;
			break;
		case 'i':
			*flags|=LS_I;
			break;
		case 'l':
			*flags|=LS_L;
			break;
		default:
			fprintf(stderr,"unknow value of -%c\n",c);
			return -1;
		}
	return 1;
}

//解析ls的输入，给出具体的ls格式
int parse_input(int argc,char **argv,char *dest_path,int *flags)
{

	for(int i=1;i<argc;i++){
		//如果带有-，则认为是ls的选项
		if(argv[i][0]=='-'&&argv[i][1]!='\0'){	
			for(int j=1;argv[i][j]!='\0';j++)
				if(__parse_flags(argv[i][j],flags)<0)
					return -1;
			
		}
		//如果不是选项，则认为是目标路径
		else{
			if(dest_path[0]!=' '){		//如果出现多个路径，则报错
				fprintf(stderr,"input format error\n");
				return -1;
			}	
			strcpy(dest_path,argv[i]);
		}
	}

	//如果没有输入路径，则默认为当前目录
	if(dest_path[0]==' ')
		strcpy(dest_path,".");

	return 1;
}

int main(int argc,char **argv)
{
	DIR *dp;
	struct dirent *dirp;
	char dest_path[MAX_PATH_LEN]={' '};
	int flags=0;

	//解析输入命令
	if(parse_input(argc,argv,dest_path,&flags)<0){
		fprintf(stderr,"format error\n");
		return -1;
	}

	//遍历目录内容
	if((dp=opendir(dest_path))==NULL){
		perror("opendir error");
		exit(0);		
	}
	
	//打印输出
	while((dirp=readdir(dp))!=NULL){
		if(dirp->d_name[0]=='.')
			continue;
		printf("%s   ",dirp->d_name);	
	}
	printf("\n");	

}
