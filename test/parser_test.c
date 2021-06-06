#include "../parser_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
        char buf[1024];
        struct cmd_list *cmd_list;
        struct cmd *cmd;

        while (1) {
                printf(">: ");
                fflush(stdout);
                fgets(buf, 1024, stdin);
                if (feof(stdin))
                        break;

                if ((cmd_list = create_cmd_list(buf)) == NULL) {
                        fprintf(stderr, "input format is error\n");
                        continue;
                }
                if (cmd_list->head != NULL) {
                        printf("command count: %d\n", cmd_list->len);
                        // print the job type
                        if (cmd_list->job_type == CMD_JOB_BG)
                                printf("job type: background job\n");
                        else if (cmd_list->job_type == CMD_JOB_FG)
                                printf("job type: forground job\n");
                        else
                                printf("job type: unknow!\n");

                        // print cmd data
                        for (cmd = cmd_list->head; cmd != NULL; cmd = cmd->next) {
                                printf("\n");
                                if (cmd->cmd_type == CMD_POSITION_EXEC)
                                        printf("command type:exec command\n");
                                else if (cmd->cmd_type == CMD_POSITION_EXTERN)
                                        printf("command type:extern command\n");
                                else if (cmd->cmd_type == CMD_POSITION_BUILTIN)
                                        printf("command type:builtin command\n");

				// print the prev command special type
                                if (cmd->prev_special_type == CMD_SPECIAL_PIPE)
                                        printf("prev special type:pipe\n");
                                else if (cmd->prev_special_type == CMD_SPECIAL_LEFT_REDIR)
                                        printf("prev special type:left redir\n");
                                else if (cmd->prev_special_type == CMD_SPECIAL_RIGHT_REDIR)
                                        printf("prev special type:right redir\n");
                                else if (cmd->prev_special_type == CMD_SPECIAL_AND)
                                        printf("prev special type:and\n");
                                else
                                        printf("prev special type:default\n");

				// print the next command special type
                                if (cmd->next_special_type == CMD_SPECIAL_PIPE)
                                        printf("next special type:pipe\n");
                                else if (cmd->next_special_type == CMD_SPECIAL_LEFT_REDIR)
                                        printf("next special type:left redir\n");
                                else if (cmd->next_special_type == CMD_SPECIAL_RIGHT_REDIR)
                                        printf("next special type:right redir\n");
                                else if (cmd->next_special_type == CMD_SPECIAL_AND)
                                        printf("next special type:and\n");
                                else
                                        printf("next special type:default\n");

                                for (int i = 0; i < cmd->argc; i++) {
                                        printf("arg%d: %s  ", i, cmd->argv[i]);
                                }
                                printf("\n");
                        }
                        destroy_cmd_list(cmd_list);
                }
        }
        exit(0);
}
