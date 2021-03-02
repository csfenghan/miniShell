#include "unix_api.h"

#define MAX_NAME_LEN 128

#define CP_R		(1<<18)
#define IS_SET_R(x) ((x)&CP_R)

//解析输入，获得输入的源路径和目的路径以及选项
int parse_input(int argc,char **argv,char **src_path,char **dest_path)
{
	int flags=0;

	for(int i=1;i<argc;i++){
		//第i个参数是选项
		if(argv[i][0]=='-'){
			for(int j=1;argv[i][j]!='\0';j++){
				switch(argv[i][j]){
					case 'r':
						flags|=CP_R;break;
					default:
						return -1;
				}
			}
		}
		//第i个参数是路径
		else{
			if(*src_path==NULL)
				*src_path=argv[i];
			else if(*dest_path==NULL)
				*dest_path=argv[i];
			else
				return -1;
		}
	}	

	return flags;
}

//获得输入的源目录和源文件名，以及目的目录和目的路径名
//src_fd和dest_fd分别是源目录和目的目录的文件描述符，*src_path和*dest_path被指向文件名（如果没有的话则为NULL)
void parse_cmd(char **src_path,char **dest_path,int *src_fd,int *dest_fd,int flags)
{
	struct stat st;	

	////////////////////////////////////////////////////////////////
	Stat(*src_path,&st);
	if(S_ISDIR(st.st_mode)){		//如果复制的是目录
		if(!IS_SET_R(flags))
			unix_error("need -r options");

		*src_fd=Open(*src_path,O_RDONLY,0);
		*src_path=NULL;	

		if(access(*dest_path,F_OK)<0)	//如果目标目录不存在，则创建
			Mkdir(*dest_path,S_IRUSR|S_IWUSR|S_IXUSR);	
		else{
			Stat(*dest_path,&st);
			if(!S_ISDIR(st.st_mode))
				unix_error("dest path is not a directory");
		}
		*dest_fd=Open(*dest_path,O_RDONLY,0);	
		*dest_path=NULL;
	}
	//////////////////////////////////////////////////////////////
	else{							//如果复制的是文件
		char *temp=strrchr(*src_path,'/');
		if(temp==NULL){		//说明是在当前目录的文件
			*src_fd=Open(".",O_RDONLY,0);
		}
		else{				
			*src_path[strlen(*src_path)-strlen(temp)-1]='\0';
			*src_fd=Open(*src_path,O_RDONLY,0);
			*src_path=temp+1;
		}

		//处理目的路径
		if(access(*dest_path,F_OK)<0){
			temp=strrchr(*dest_path,'/');
			if(temp==NULL)
				unix_error("error");
			*dest_path[strlen(*dest_path)-strlen(temp)-1]='\0';
			if(strcmp(*dest_path,"/home/fenghan/miniShell")==0)
				printf("%s\n",*dest_path);
			*dest_fd=Open(*dest_path,O_RDONLY,0);
			*dest_path=temp+1;
			printf("%s\n",*dest_path);
		}
		else{
			struct stat st;
			Stat(*dest_path,&st);
			if(S_ISDIR(st.st_mode)){
				*dest_fd=Open(*dest_path,O_RDONLY,0);
				*dest_path=*src_path;
			}
			else{
				temp=strrchr(*dest_path,'/');
				if(temp==NULL)
					*dest_fd=Open(".",O_RDONLY,0);
				else{
					*dest_path[strlen(*dest_path)-strlen(temp)-1]='\0';
					*dest_fd=Open(*dest_path,O_RDONLY,0);
					*dest_path=temp+1;
				}
			}
		}
	}
}

void copy(int src_fd,int dest_fd,char *src_path,char *dest_path)
{

}

int main(int argc,char **argv)
{
	char src_path[1024]={},dest_path[1024]={};
	int src_fd,dest_fd;
	int flags;

	if(strcmp(argv[0],"cp")!=0&&strcmp(argv[0],"./cp")!=0)
		unix_error("error usage of cp");

	umask(0);

	if((flags=parse_input(argc,argv,src_path,dest_path))<0)
		unix_error("invalid option");

	parse_cmd(&src_path,&dest_path,&src_fd,&dest_fd,flags);
	
	printf("src_path:%s,dest_path:%s,flags:%d\n",src_path,dest_path,flags);

	exit(0);
}
