#include "builtin.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (cmd == NULL)
		return 0;

	if (strncmp(cmd, "exit", 4) == 0)
		return 1;

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd == NULL || strncmp(cmd, "cd", 2) != 0)
		return 0;

	cmd += 2;

	while (*cmd == ' ')
		cmd++;

	char *path = NULL;
	if (*cmd == '\0' || *cmd == '\n') {
		path = getenv("HOME");
		if (path == NULL) {
			printf_debug("Error al obtener el path de HOME.\n");
			return 1;
		}
	} else {
		path = cmd;
	}

	if (chdir(path) != 0)
		printf_debug("Error al cambiar de directorio.\n");

	char cwd[BUFLEN];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		snprintf(prompt, sizeof(prompt), "%s", cwd);
	} else {
		printf_debug("Error en getcwd.\n");
		return 1;
	}

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (cmd == NULL || strncmp(cmd, "pwd", 3) != 0)
		return 0;

	char *cwd = getcwd(NULL, 0);
	if (cwd == NULL) {
		printf_debug("Error en getcwd.\n");
		return 1;
	}

	printf("%s\n", cwd);
	free(cwd);
	return 1;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	if (cmd == NULL || strncmp(cmd, "history", 7) != 0)
		return 0;

	printf("History feature not implemented.\n");
	return 1;
}
