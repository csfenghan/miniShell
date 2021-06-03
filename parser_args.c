#include "mini_shell.h"
#include <unix_api.h>

/* 解析输入，以空格为分界，将其划分为数组格式。如果是后台命令则返回1，否则默认前台目录返回0
 */
int parse_line(char *buf, char **argv) {
        char *delim;
        int argc;
        int bg = 0; //是否是后台任务

        buf[strlen(buf) - 1] = ' ';
        while (*buf && (*buf == ' '))
                buf++;

        argc = 0;
        while ((delim = strchr(buf, ' '))) {
                argv[argc++] = buf;
                *delim = '\0';
                buf = delim + 1;
                while (*buf && (*buf == ' '))
                        buf++;
        }
        argv[argc] = NULL;

        if (argc == 0)
                return 1;

        if ((bg = (*argv[argc - 1] == '&')) != 0) //是否是后台任务
                argv[--argc] = NULL;

        return bg;
}
/**********************************************************************
 *	新版本
 *********************************************************************/

// job的类型
enum { CMD_JOB_BG= 1,
       CMD_JOB_FG= 2,
};

// 命令的类型
enum { CMD_POSITION_BUILTIN = 1, CMD_POSITION_EXTERN = 2, CMD_POSITION_OTHER = 3 };

struct arg_list_node {};

struct cmd {
        int argc;     //命令的长度
        char **argv;  //命令
        int job_type; //是否是后台任务
        int cmd_type; //是根据目录来进行执行命令，还是shell内置或bin中的命令
};

// 根据输入的buf来解析命令的格式，并动态分配内存
// 想要动态的管理内存，可以使用链表
struct cmd *create_cmd(char *buf) {
        struct cmd *result;
        char *delim;
        char *ptr;

        result = Malloc(sizeof(struct cmd));

        // 最后一个是换行符，将其改成空格以方便解析
        ptr = buf;
        buf[strlen(ptr) - 1] = ' ';
        while (*ptr && (*ptr == ' '))
                ptr++;

        // 计算参数的个数，为result->argv分配内存
        while ((delim = strchr(ptr, ' '))) {
                result->argc++;
                ptr = delim + 1;
                while (*ptr && (*ptr == ' '))
                        ptr++;
        }
        result->argv = Malloc(sizeof(char *) * result->argc);

        // 挨个解析命令，存入result->argv中
        ptr = buf;
        for (int i = 0; i < result->argc; i++) {
                delim = strchr(ptr, ' ');
                result->argv[i] = ptr;
                *delim = '\0';
                ptr = delim + 1;
                while (*ptr && (*ptr == ' '))
                        ptr++;
        }

	if(result->argc==0)
		return result;

	// 解析命令的位置
	char *front=result->argv[0];
	if(strlen(front)>2 && front[0]=='.' && front[1]=='/')
		result->cmd_type=CMD_POSITION_EXTERN;
	else
		result->cmd_type=CMD_POSITION_OTHER;

	// 解析job的类型
        char *back = result->argv[result->argc - 1];
        if (back[strlen(back) - 1] == '&') {
                result->job_type = CMD_JOB_BG;
                back[strlen(back) - 1] = '\0';
        } else {
                result->job_type = CMD_JOB_FG;
        }

	return result;
}

int destroy_cmd(struct cmd *cmd) {
	free(cmd->argv);
	free(cmd);

	return 1;
}
