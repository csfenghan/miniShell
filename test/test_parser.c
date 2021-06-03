#include "../parser_args.h"
#include <stdio.h>
#include <unistd.h>

int main() {
        char buf[1024];
        struct cmd_list *cmd_list;
        struct cmd *cmd;

        while (1) {
                printf(">: ");
                fflush(stdout);
                fgets(buf, 1024, stdin);

                if ((cmd_list = create_cmd_list(buf)) == NULL) {
                        fprintf(stderr, "input format is error\n");
                        continue;
                }
                if (cmd_list->head != NULL) {
                        printf("command count: %d\n", cmd_list->len);
                        // print the job type
                        if (cmd_list->job_type == CMD_JOB_BG) {
                                printf("job type: background job\n");
                        } else if (cmd_list->job_type == CMD_JOB_FG) {
                                printf("job type: forground job\n");
                        } else {
                                printf("job type: unknow!\n");
                        }

                        // print cmd data
                        for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
                                if (cmd->cmd_type == CMD_POSITION_OTHER)
                                        printf("    command type:other command\n");
                                else if (cmd->cmd_type == CMD_POSITION_EXTERN)
                                        printf("    command type:extern command\n");
                                else if (cmd->cmd_type == CMD_POSITION_BUILTIN)
                                        printf("    command type:builtin command\n");

                                if (cmd->special_type == CMD_SPECIAL_PIPE)
                                        printf("    special type:pipe\n");
                                else if (cmd->special_type == CMD_SPECIAL_LEFT_REDIR)
                                        printf("    special type:left redir\n");
                                else if (cmd->special_type == CMD_SPECIAL_RIGHT_REDIR)
                                        printf("    special type:right redir\n");
                                else if (cmd->special_type == CMD_SPECIAL_AND)
                                        printf("    special type:and\n");
				else
					printf("    special type:def\n");

                                printf("        ");
                                for (int i = 0; i < cmd->argc; i++) {
                                        printf("arg%d: %s  ", i, cmd->argv[i]);
                                }
                                printf("\n");
                        }
			destroy_cmd_list(cmd_list);
                }
        }
}
