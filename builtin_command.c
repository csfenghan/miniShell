// this file contains the implementation of the built-in commands
// and provides the interface to manipulate built-in commands

#include "builtin_command.h"
#include "job_manger.h"

static void *builtin_cd(int argc, char **argv);
static void *builtin_jobs(int argc, char **argv);
static void *builtin_fg(int argc, char **argv);
static void *builtin_bg(int argc, char **argv);
static void *builtin_pwd(int argc, char **argv);
static void *builtin_echo(int argc, char **argv);
static void *builtin_test(int argc,char **argv);

/**********************************************************
 *	arrays for saving built-in commands and their interface
 *********************************************************/
struct builtin_cmd {
        char *name;
        void *(*func)(int argc, char **argv);
};
struct builtin_cmd builtin_cmds[] = {{"cd", builtin_cd},
                                     {"pwd", builtin_pwd},
                                     {"jobs", builtin_jobs},
                                     {"fg", builtin_fg},
                                     {"bg", builtin_bg},
				     {"echo",builtin_echo},
				     {"test",builtin_test},
};

// search the command and return index of builtin_cmds if the command is builtin,
// return -1 if the command not builtin
int search_builtin_command(char *name) {
        int count;
        int i;

        count = sizeof(builtin_cmds) / sizeof(builtin_cmds[0]);
        for (i = 0; i < count; i++) {
                if (strcmp(name, builtin_cmds[i].name) == 0)
                        return i;
        }
        return -1;
}

// exec the builtin command.return 1 if comand exec successfully,return -1 if the command not
// builtin
int exec_builtin_command(char *name, int argc, char **argv) {
        int index;
        void *(*func)(int, char **);

        index = search_builtin_command(name);
        if (index < 0)
                return -1;
        func = builtin_cmds[index].func;
        func(argc, argv);
        return 1;
}

/*********************************************************************
 *	the concrete implementation of built-in commands
 ********************************************************************/
static void *builtin_cd(int argc, char **argv) {
        if (argc > 2) {
                fprintf(stderr, "too many arguments\n");
                return (void *)-1;
        }
        char *path;

        path = (argc == 1) ? getenv("HOME") : argv[1];
        if (chdir(path) < 0)
                fprintf(stderr, "chdir %s error:%s\n", path, strerror(errno));
        return (void *)1;
}

static void *builtin_pwd(int argc, char **argv) {
        if (argc > 1) {
                fprintf(stderr, "too many arguments\n");
                return (void *)-1;
        }
        printf("%s\n", getcwd(NULL, 0));
        return (void *)1;
}

static void *builtin_jobs(int argc, char **argv) {
	list_job();
        return (void *)1;
}
static void *builtin_fg(int argc, char **argv) {
        //do_fgbg(argv);
        return (void *)1;
}
void *builtin_bg(int argc, char **argv) {
        //do_fgbg(argv);
        return (void *)1;
}

static void *builtin_echo(int argc, char **argv) {
        for (int i = 1; i < argc; i++)
		write(STDOUT_FILENO,argv[i],strlen(argv[i]));
	write(STDOUT_FILENO,"\n",1);
        return (void *)1;
}
static void *builtin_test(int argc,char **argv){
	char buf[MAXLINE];

	read(STDIN_FILENO,buf,MAXLINE);
	write(STDOUT_FILENO,"result of test:",15);
	write(STDOUT_FILENO,buf,strlen(buf));

	return (void *)1;
}
