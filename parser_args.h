#ifndef PARSER_ARGS_H
#define PARSER_ARGS_H

// job的类型
enum job_type {
        CMD_JOB_FG = 0,
        CMD_JOB_BG = 1,
};

// 命令的类型
enum cmd_type {
        CMD_POSITION_UNKNOW= 0,
        CMD_POSITION_EXEC = 1,
        CMD_POSITION_BUILTIN = 2,
        CMD_POSITION_EXTERN = 3,
};

// 特殊类型（如重定向、管道等）
enum special_type {
        CMD_SPECIAL_DEFAULT = 0,
        CMD_SPECIAL_LEFT_REDIR = 1,
        CMD_SPECIAL_RIGHT_REDIR = 2,
        CMD_SPECIAL_PIPE = 3,
        CMD_SPECIAL_AND = 4,
};

struct cmd {
        int argc;               //命令的长度
        char **argv;            //命令
        enum cmd_type cmd_type; //是根据目录来进行执行命令，还是shell内置或bin中的命令

        enum special_type prev_special_type; //后面是否有管道等特殊符号，只有当next不为NULL时才有效
	enum special_type next_special_type; //前面的命令是否使用管道等特殊符号
        struct cmd *next; //指向下一个cmd
};

// 解析的命令组，如cat file.c | grep test会被解析为两个struct cmd类型
struct cmd_list {
        int len;                //列表的长度
        enum job_type job_type; //是否是后台任务
        struct cmd *head;
        struct cmd *tail;
};

void destroy_cmd_list(struct cmd_list *cmd_list);
struct cmd_list *create_cmd_list(char *buf);

#endif
