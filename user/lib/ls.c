#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

int main(int argc,char **argv)
{
	DIR *dp;
	struct dirent *dirp;

	if(argc==1)
		argv[1]=".";

	if((dp=opendir(argv[1]))==NULL){
		perror("opendir error");
		exit(0);		
	}
	
	while((dirp=readdir(dp))!=NULL){
		if(dirp->d_name[0]=='.')
			continue;
		printf("%s   ",dirp->d_name);	
	}
	printf("\n");	

}
