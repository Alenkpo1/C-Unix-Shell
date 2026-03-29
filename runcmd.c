#include "runcmd.h"
#include "parsing.h"
#include "builtin.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdbool.h>

int status = 0;
struct cmd *parsed_pipe;

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	size_t len = strlen(cmd);
	if (len > 0 && cmd[len - 1] == '\n')
		cmd[len - 1] = '\0';

	char *s = cmd;
	while (*s == ' ' || *s == '\t')
		s++;
	if (*s == '\0')
		return 0;

	// "history" built-in call
	if (history(s))
		return 0;

	// "cd" built-in call
	if (cd(s))
		return 0;

	// "exit" built-in call
	if (exit_shell(s))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(s))
		return 0;

	// parses the command line
	parsed = parse_line(s);

	// forks and run the command
	p = fork();
	if (p < 0) {
		printf_debug("Error en fork.\n");
		return 0;
	}

	if (p == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		if (setpgid(0, 0) < 0) {
			printf_debug("Error en setpgid.\n");
			_exit(1);
		}
		exec_cmd(parsed);
		free_command(parsed);
		_exit(1);
	}

	// stores the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'
	//
	if (parsed->type == BACK) {
		print_back_info(parsed);
	} else {
		// waits for the process to finish
		waitpid(p, &status, 0);

		print_status_info(parsed);
	}

	free_command(parsed);
	return 0;
}
