#include "unix_api.h"
#include <grp.h>
#include <pwd.h>

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

//一些奇怪但不得不加的声明
int fstatat(int,const char *restrict,struct stat *,int);
void __parse_flags(char,int*);
void __print_line(int fd,const char *pathname,int flags);

//按照标志的类别添加flags
inline void __parse_flags(char c,int *flags)
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
			unix_error("unknow value");
		}
}

//根据flags的值显示一个文件的输出
//ls -l格式：-rw-r--r-- 1 fenghan fenghan   304 Feb 27 08:52 Makefile
void __print_line(int fd,const char *pathname,int flags)
{
	struct stat st;
	struct passwd *pw;
	struct group *gp;

	if(fstatat(fd,pathname,&st,0)<0)
		unix_error("fstatat error");
	
	//是否隐藏文件
	if(!IS_SET_A(flags)){
		if(pathname[0]=='.')
			return;
	}
	//依次显示文件类型、权限、link值、所有者、所属组、大小、最后修改日期和文件名
	if(IS_SET_L(flags)){
		//显示inode值
		if(IS_SET_I(flags)){
			printf("%6d ",(int)st.st_ino);
		}

		//显示文件类型
		if(S_ISREG(st.st_mode))
			printf("-");
		else if(S_ISDIR(st.st_mode))
			printf("d");
		else if(S_ISCHR(st.st_mode))
			printf("c");
		else if(S_ISLNK(st.st_mode))
			printf("l");
		else
			printf(" ");
		
		//显示文件权限
		printf("%c",(st.st_mode&S_IRUSR)?'r':'-');
		printf("%c",(st.st_mode&S_IWUSR)?'w':'-');
		printf("%c",(st.st_mode&S_IXUSR)?'x':'-');

		printf("%c",(st.st_mode&S_IRGRP)?'r':'-');
		printf("%c",(st.st_mode&S_IWGRP)?'w':'-');
		printf("%c",(st.st_mode&S_IXGRP)?'x':'-');

		printf("%c",(st.st_mode&S_IROTH)?'r':'-');
		printf("%c",(st.st_mode&S_IWOTH)?'w':'-');
		printf("%c ",(st.st_mode&S_IXOTH)?'x':'-');


		//显示链接数
		printf("%3d ",(int)st.st_nlink);

		//显示所属者和所属组
		pw=getpwuid(st.st_uid);
		printf("%8s ",pw->pw_name);
		
		gp=getgrgid(st.st_gid);
		printf("%8s ",gp->gr_name);

		//显示文件大小
		printf("%9d ",(int)st.st_size);

		//显示最后修改时间
		
		//显示文件名
		printf("%s",pathname);
		printf("\n");	
	}
	//将文件名全部输出到一行
	else{
		if(IS_SET_I(flags)){
			printf("%d ",(int)st.st_ino);
		}
		printf("%s   ",pathname);	
	}
}

//解析ls的输入
//dest_path：要显示的目录
//flags：ls的选项
void parse_input(int argc,char **argv,char *dest_path,int *flags)
{

	for(int i=1;i<argc;i++){
		//如果带有-，则认为是ls的选项
		if(argv[i][0]=='-'&&argv[i][1]!='\0'){	
			for(int j=1;argv[i][j]!='\0';j++)
				__parse_flags(argv[i][j],flags);
		}
		//如果不是选项，则认为是目标路径
		else{
			if(dest_path[0]!=' ')			//如果出现多个路径，则报错
				unix_error("input format error");

			strcpy(dest_path,argv[i]);
		}
	}

	//如果没有输入路径，则默认为当前目录
	if(dest_path[0]==' ')
		strcpy(dest_path,".");
}

//打印输出
//ls -l格式：-rw-r--r-- 1 fenghan fenghan   304 Feb 27 08:52 Makefile
void print_result(const char *dest_path,int flags)
{
	int fd;
	DIR *dp;
	struct dirent *dirp;

	access(dest_path,F_OK);
	dp=Opendir(dest_path);
	fd=Open(dest_path,O_RDONLY,0);

	//格式化打印输出行
	while((dirp=readdir(dp))!=NULL)
		__print_line(fd,dirp->d_name,flags);	
	
	printf("\n");	
}

int main(int argc,char **argv)
{
	char dest_path[MAX_PATH_LEN]={' '};
	int flags=0;

	parse_input(argc,argv,dest_path,&flags);
	print_result(dest_path,flags);

	exit(0);
}
