#include "unix_api.h"

#define MAX_PATH_LEN 128

#define CP_R		(1<<18)
#define IS_SET_R(x) ((x)&CP_R)

void __parser_flags(char c,int *flags);


inline void __parser_flags(char c,int *flags)
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
//example:
//		input:argc=4,argv={"cp","-r","/home/my/a.txt","/home/my/b.txt"}
//		output:src_path="/home/my/a.txt",dest_path="/home/my/b.txt",*flags=*flags|CP_R
//
void parse_input(int argc,char **argv,char *src_path,char *dest_path,int *flags)
{
	if(strcmp(argv[0],"cp")!=0)
		unix_error("input format error");
	
	for(int i=1;i<argc;i++){
		//如果带有-，则认为是option
		if(argv[i][0]=='-'){
			for(int j=1;argv[i][j]!='\0';j++)
				__parser_flags(argv[i][j],flags);
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
}

//将文件src_path复制为dest_path或复制到dest_path路径
void copy_file(char *src_path,char *dest_path,int flags)
{
	struct stat st;

	Access(src_path,F_OK);
	Stat(src_path,&st);

	umask(0);
	if(S_ISDIR(st.st_mode)){
		if(IS_SET_R(flags)){

		}
		else{

		}
	}

	//非目录
	else{
		int src_fd,dest_fd;
		char buf[4096];
		int n;

		src_fd=Open(src_path,O_RDONLY,0);
		dest_fd=Open(dest_path,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

		while((n=Read(src_fd,buf,sizeof(buf)))>0){
			Write(dest_fd,buf,n);
		}
	}
}

int main(int argc,char **argv)
{
	char src_path[MAX_PATH_LEN]={};
	char dest_path[MAX_PATH_LEN]={};
	int flags=0;

	parse_input(argc,argv,src_path,dest_path,&flags);
	copy_file(src_path,dest_path,flags);

	exit(0);
}
