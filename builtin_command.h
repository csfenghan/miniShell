// this file provides the interface to manipulate built-in commands

#ifndef BUILTIN_COMMAND_H
#define BUILTIN_COMMAND_H

// search the command and return index of builtin_cmds if the command is builtin,
// return -1 if the command not builtin
int search_builtin_command(char *name);

// exec the builtin command.return 1 if comand exec successfully,return -1 if the command not
// builtin
int exec_builtin_command(char *name, int argc, char **argv);


#endif
