#include "unix_api.h"

#define MAX_PATH_LEN 128

#define CP_R		(1<<18)
#define IS_SET_R(x) ((x)&CP_R)

void parser_flags(char c,int *flags);


inline void parser_flags(char c,int *flags)
{
	switch(c){
		case 'r':
			*flags|=CP_R;
			break;

		default:
			unix_error("invalid option");
			break;
	}
}

//解析输入命令
//example1:
//		input:argc=4,argv={"cp","-r","/home/my/a.txt","/home/my/b.txt"}
//		output:src_path="/home/my",file_name=NULL,dest_path="/home/my/b.txt",*flags=*flags|CP_R
//exampl2:
//		input:argc=3,argv={"cp","/home/my/foo.txt","/home/my/temp/"}
//		output:src_path="/home/my",file_name="foo.txt",dest_path="/home/my/temp/foo.txt"
//
void parse_input(int argc,char **argv,char *src_path,char *src_name,char *dest_path,char *dest_name,int *flags)
{
	char *temp;

	if(strcmp(argv[0],"cp")!=0)
		unix_error("input format error");
	
	//解析输入
	for(int i=1;i<argc;i++){
		//如果带有-，则认为是option
		if(argv[i][0]=='-'){
			for(int j=1;argv[i][j]!='\0';j++)
				parser_flags(argv[i][j],flags);
		}		
		//否则认为是路径
		else{
			if(src_path[0]=='\0')			//遇到的第一个路径
				strcpy(src_path,argv[i]);
			else if(dest_path[0]=='\0')		//遇到的第二个路径
				strcpy(dest_path,argv[i]);
			else							//路径超过两个
				unix_error("too many arguments");
		}
	}	

	if(src_path[0]=='\0'||dest_path[0]=='\0')
		unix_error("missing file operand");

	//判断输入的源路径是目录还是文件，如果是目录，则file_name=NULL
	temp=strrchr(src_path,'/');	
	if(strcmp(temp,"/")==0){		//是目录
		temp=NULL;
		src_path[strlen(src_path)-1]='\0';
	}		
	else{
		struct stat st;

		Stat(src_path,&st);
		if(S_ISDIR(st.st_mode))	//是目录
			temp=NULL;		
		else{					//如果不是目录，则将src_path改为目录，file_name为文件名
			temp++;	
			src_path[strlen(src_path)-strlen(temp)-1]='\0';
		}
	}
	strcpy(src_name,temp);
}

//将文件src_path复制为dest_path或复制到dest_path路径
void copy_file(char *src_path,char *file_name,char *dest_path,int flags)
{
	struct stat st;
	char buf[4096];
	int dir_fd,src_fd,dest_fd,n;

	//如果没有指定文件名，则默认为src_path的文件名	
	if(access(dest_path,F_OK)==0){		
		Stat(dest_path,&st);	
		if(S_ISDIR(st.st_mode)){		//如果文件存在且是目录，则默认被复制的文件名即复制后的文件名

			if(dest_path[strlen(dest_path)-1]!='/')
				strcat(dest_path,"/");

			strcat(dest_path,file_name);
		}
	}

	//开始复制
	
	umask(0);
	dir_fd=Open(src_path,O_RDONLY,0);
	src_fd=Openat(dir_fd,src_path,O_RDONLY,0);
	dest_fd=Open(dest_path,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	while((n=Read(src_fd,buf,sizeof(buf)))>0)
		Write(dest_fd,buf,n);
}

//复制一个目录
void copy_dir(char *src_path,char *dest_path,int flags)
{
	DIR *dp;
	struct dirent *drip;
	char *file_name,temp;

	dp=Opendir(src_path);
	while((drip=Readdir(dp))!=NULL){
		
	}

}

//执行复制任务
void copy(char *src_path,char *file_name,char *dest_path,int flags)
{
	Access(src_path,F_OK);
	//如果被复制的是目录
	if(file_name==NULL){
		if(IS_SET_R(flags))
			copy_dir(src_path,dest_path,flags);
		else
			unix_error("cp: -r not specified; omitting directory");
	}
	else
		copy_file(src_path,file_name,dest_path,flags);
}

int main(int argc,char **argv)
{
	char src_path[MAX_PATH_LEN]={};
	char src_name[MAX_PATH_LEN]={};
	char dest_path[MAX_PATH_LEN]={};
	char dest_name[MAX_PATH_LEN]={};
	int flags=0;

	parse_input(argc,argv,src_path,src_name,dest_path,dest_name,&flags);
	copy(src_path,src_name,dest_path,flags);

	exit(0);
}
