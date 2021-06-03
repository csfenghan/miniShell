#include "parser_args.h"
#include <unix_api.h>

// parser the input,if command is the background,return 1,else reurn 0
int parse_line(char *buf, char **argv) {
        char *delim;
        int argc;
        int bg = 0;

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

        if ((bg = (*argv[argc - 1] == '&')) != 0)
                argv[--argc] = NULL;

        return bg;
}
/**********************************************************************
 *	new version
 *********************************************************************/
// free the memory of struct cmd
static void destroy_cmd(struct cmd *cmd) {
        free(cmd->argv);
        free(cmd);
}

// parser the input,return pointer of struct cmd
static struct cmd *create_cmd(char *buf) {
        struct cmd *result;
        char *delim;
        char *ptr;

        if (strlen(buf) == 0)
                return NULL;

        result = Malloc(sizeof(struct cmd));

        // replace '\n' with ' ' to facilitate parsing
        ptr = buf;
        buf[strlen(ptr) - 1] = ' ';
        while (*ptr && (*ptr == ' '))
                ptr++;

        // calculate the numbe of args,and malloc the memory for result->argv
        while ((delim = strchr(ptr, ' '))) {
                result->argc++;
                ptr = delim + 1;
                while (*ptr && (*ptr == ' '))
                        ptr++;
        }
        result->argv = Malloc(sizeof(char *) * result->argc);

        // parse the input,save the results to result->argv
        ptr = buf;
        for (int i = 0; i < result->argc; i++) {
                delim = strchr(ptr, ' ');
                result->argv[i] = ptr;
                *delim = '\0';
                ptr = delim + 1;
                while (*ptr && (*ptr == ' '))
                        ptr++;
        }

        if (result->argc == 0) {
                destroy_cmd(result);
                return NULL;
        }

        // judge the command format is ./cmd or cmd
        char *front = result->argv[0];
        if (strlen(front) > 2 && front[0] == '.' && front[1] == '/')
                result->cmd_type = CMD_POSITION_EXTERN;
        else
                result->cmd_type = CMD_POSITION_OTHER;

        return result;
}

// release the cmd_list
void destroy_cmd_list(struct cmd_list *cmd_list) {
        struct cmd *curr;

        while (cmd_list->head != NULL) {
                curr = cmd_list->head->next;
                destroy_cmd(cmd_list->head);
                cmd_list->head = curr;
        }
        free(cmd_list);
}

// parse the input
struct cmd_list *create_cmd_list(char *buf) {
        // find the special characters:such as |,<,>,&&
        struct cmd *curr;
        struct cmd_list *result;
        char *left, *right;

        result = Malloc(sizeof(struct cmd_list));
        result->head = NULL;
        result->tail = NULL;
        result->len = 0;

        left = buf;
        right = buf;
        while (*right) {
                // determine if there are special characters
                if ((*right == '|') || (*right == '<') || (*right == '>') ||
                    ((*right == '&') && (*(right + 1) == '&'))) {

                        char c = *right;

                        *right = '\0';
                        if ((curr = create_cmd(left)) == NULL) {
                                destroy_cmd_list(result);
                                return NULL;
                        }

                        // select the type according to the character
                        switch (c) {
                        case '|':
                                curr->special_type = CMD_SPECIAL_PIPE;
                                break;
                        case '<':
                                curr->special_type = CMD_SPECIAL_LEFT_REDIR;
                                break;
                        case '>':
                                curr->special_type = CMD_SPECIAL_RIGHT_REDIR;
                                break;
                        case '&':
                                curr->special_type = CMD_SPECIAL_AND;
                                right++;
                                break;
                        }

                        // inserting a list node
                        curr->next = NULL;
                        if (result->head == NULL) {
                                result->head = curr;
                                result->tail = curr;
                        } else {
                                result->tail->next = curr;
                                result->tail = curr;
                        }
                        result->len++;

                        left = ++right;
                        continue;
                }
                right++;
        }
        // parser again when end the loop
        result->len++;
        if ((curr = create_cmd(left)) == NULL) {
                destroy_cmd_list(result);
                return NULL;
        }
        curr->next = NULL;
        if (result->head == NULL) {
                result->head = curr;
                result->tail = curr;
        } else {
                result->tail->next = curr;
                result->tail = curr;
        }

        return result;
}
