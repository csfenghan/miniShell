#include "unix_api.h"

#define MAX_PATH_LEN 1024
#define CP_R    (1<<('r'-'a'))
#define IS_SET_R(x) ((x)&CP_R)

//解析argv参数
void parse_input(int argc,char **argv,char *src_path,char *dest_path,int *flags)
{
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){    //以-开头，代表选项
            for(int j=1;j<strlen(argv[i]);j++)
                switch(argv[i][j]){
                    case 'r':   *flags|=CP_R;break;
                    default:    unix_error("invalid option");
                }
        }else{                  //否则默认是路径
            if(src_path[0]=='\0')
                strcpy(src_path,argv[i]);
            else if(dest_path[0]=='\0')
                strcpy(dest_path,argv[i]);
            else
                unix_error("cp: too many arguments");
        }
    }

    if(dest_path[0]=='\0')
        unix_error("cp: missing file operand");
}

void parse_src_path(char *src_path,char *src_file,int *is_dir)
{
    struct stat st;
    char *name;    
    //解析源路径
    Stat(src_path,&st);
    *is_dir=S_ISDIR(st.st_mode)?1:0;

    if(src_path[strlen(src_path)-1]=='/')
        src_path[strlen(src_path)-1]='\0';
    
    name=strrchr(src_path,'/');
    if(name==NULL){
        strcpy(src_file,src_path);
        strcpy(src_path,".");
    }else{
        strcpy(src_file,++name);
        src_path[strlen(src_path)-strlen(name)]='\0';
    }
}

void parse_dest_path(char *dest_path,char *dest_file,char *src_file,int *is_dir)
{
    char *name;

    if(dest_path[strlen(dest_path)-1]=='/')
        dest_path[strlen(dest_path)-1]='\0';

    //解析目的路径
    if(access(dest_path,F_OK)==0){
        struct stat st;
        Stat(dest_path,&st);

        if(S_ISDIR(st.st_mode))
            strcpy(dest_file,src_file);
        else{
            name=strrchr(dest_path,'/');
            if(name==NULL){
                strcpy(dest_file,dest_path);
                strcpy(dest_path,".");
            }else{
                strcpy(dest_file,++name);
                dest_path[strlen(dest_path)-strlen(name)]='\0';
            }
        }
    }else{                          //如果目的位置不存在
        name=strrchr(dest_path,'/');
        if(name==NULL){
            strcpy(dest_file,dest_path);
            strcpy(dest_path,".");
        }else{
            strcpy(dest_file,++name);
            dest_path[strlen(dest_path)-strlen(name)]='\0';
        }
    }
}

//解析要输入的源路径源文件和目的路径和目的文件
void parse_path(char *src_path,char *src_file,char *dest_path,char *dest_file,int *is_dir)
{
    parse_src_path(src_path,src_file,is_dir);
    parse_dest_path(dest_path,dest_file,src_file,is_dir);
}
void copy_file(int src_dir_fd,char *src_file,int dest_dir_fd,char *dest_file)
{
    int src_file_fd,dest_file_fd;
    char buf[4096];
    int n;

    src_file_fd=Openat(src_dir_fd,src_file,O_RDONLY,0);
    dest_file_fd=Openat(dest_dir_fd,dest_file,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    while((n=Read(src_file_fd,buf,4096))>0)
        Write(dest_file_fd,buf,n);
}

void copy_dir(int src_dir_fd,char *src_file,int dest_dir_fd,char *dest_file)
{
    DIR *dp;
    struct dirent *dirp;
    struct stat st;
    int src_file_fd,dest_file_fd;

    if(faccessat(dest_dir_fd,dest_file,F_OK,0)!=0)
        Mkdirat(dest_dir_fd,dest_file,S_IRUSR|S_IWUSR|S_IXUSR);

    src_file_fd=Openat(src_dir_fd,src_file,O_RDONLY,0);
    dest_file_fd=Openat(dest_dir_fd,dest_file,O_RDONLY,0);
    dp=Fdopendir(src_file_fd);

    //默认src_file是目录
    while((dirp=Readdir(dp))!=NULL){
        Fstatat(src_file_fd,dirp->d_name,&st,0); 
        if(S_ISDIR(st.st_mode)){
            if(strcmp(dirp->d_name,".")==0||strcmp(dirp->d_name,"..")==0)
                continue;
            copy_dir(src_file_fd,dirp->d_name,dest_file_fd,dirp->d_name);
        }else
            copy_file(src_file_fd,dirp->d_name,dest_file_fd,dirp->d_name);
    } 
}
void start_copy(char *src_path,char *src_file,char *dest_path,char *dest_file,int is_dir,int flags)
{
    if(is_dir&&(!IS_SET_R(flags)))
        unix_error("cp: -r not specified");

    int src_dir_fd,dest_dir_fd;

    src_dir_fd=Open(src_path,O_RDONLY,0);
    dest_dir_fd=Open(dest_path,O_RDONLY,0);

    if(is_dir)
        copy_dir(src_dir_fd,src_file,dest_dir_fd,dest_file);
    else
        copy_file(src_dir_fd,src_file,dest_dir_fd,dest_file);
}


int main(int argc,char **argv)
{
    char src_path[MAX_PATH_LEN]={'\0'},dest_path[MAX_PATH_LEN]={'\0'};
    char src_file[64]={'\0'},dest_file[64]={'\0'};
    int is_dir=0,flags=0;

    umask(0);

    parse_input(argc,argv,src_path,dest_path,&flags);
    parse_path(src_path,src_file,dest_path,dest_file,&is_dir);
    start_copy(src_path,src_file,dest_path,dest_file,is_dir,flags);

    exit(0);
}
