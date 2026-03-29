#include "exec.h"

extern int status;
void exec_final_cmd(struct cmd *cmd);

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int indice = block_contains(eargv[i], '=');
		if (indice != -1) {
			int tamanio_key = indice + 1;
			char *key = malloc(sizeof(char) * tamanio_key);
			if (key == NULL)
				return;
			int tamanio_value = strlen(eargv[i]) - indice + 1;
			char *value = malloc(sizeof(char) * tamanio_value);
			if (value == NULL) {
				free(key);
				return;
			}

			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, indice);

			setenv(key, value, 1);

			free(key);
			free(value);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd = open(file, flags, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		printf_debug("Error en open.\n");
		_exit(1);
	}
	return fd;
}

void
exec_final_cmd(struct cmd *cmd)
{
	struct execcmd *e;
	struct execcmd *r;

	switch (cmd->type) {
	case EXEC:
		e = (struct execcmd *) cmd;
		if (e->argv[0] == NULL)
			_exit(0);
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		printf_debug("Error en execvp.\n");
		_exit(1);
		break;

	case REDIR:
		r = (struct execcmd *) cmd;

		if (strlen(r->in_file) > 0) {
			int fd_in = open_redir_fd(r->in_file, O_RDONLY);
			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		if (strlen(r->out_file) > 0) {
			int fd_out = open_redir_fd(r->out_file,
			                           O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		if (strlen(r->err_file) > 0) {
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				int fd_err = open_redir_fd(r->err_file,
				                           O_WRONLY | O_CREAT |
				                                   O_TRUNC);
				dup2(fd_err, STDERR_FILENO);
				close(fd_err);
			}
		}
		set_environ_vars(r->eargv, r->eargc);
		execvp(r->argv[0], r->argv);
		printf_debug("Error en execvp.\n");
		_exit(1);
		break;

	default:
		fprintf_debug(stderr,
		              "Unsupported command type in exec_final_cmd\n");
		_exit(1);
		break;
	}
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		e = (struct execcmd *) cmd;
		if (e->argv[0] == NULL)
			return;

		exec_final_cmd(cmd);
		_exit(1);
		break;
	case BACK:
		// runs a command in background
		b = (struct backcmd *) cmd;
		pid_t pid = fork();

		if (pid == 0) {
			pid_t ppid = getppid();
			setpgid(0, 0);

			exec_cmd(b->c);
			_exit(0);
		} else if (pid > 0) {
			printf_debug("Proceso en segundo plano creado. PID del "
			             "hijo=%d\n",
			             pid);
		} else {
			printf_debug("Error en fork.\n");
		}
		break;
	case REDIR:
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		pid_t pid_redir = fork();
		if (pid_redir == 0) {
			exec_final_cmd(cmd);
		} else if (pid_redir > 0) {
			waitpid(pid_redir, &status, 0);
		} else {
			printf_debug("Error en fork.\n");
		}
		break;
	case PIPE:
		// pipes two commands
		p = (struct pipecmd *) cmd;
		int fds[2];
		if (pipe(fds) < 0) {
			printf_debug("Error en pipe.\n");
			_exit(1);
		}

		pid_t left_pid = fork();
		if (left_pid == 0) {
			close(fds[0]);
			dup2(fds[1], STDOUT_FILENO);
			close(fds[1]);
			exec_cmd(p->leftcmd);
			free_command((struct cmd *) p);
			_exit(1);
		}

		pid_t right_pid = fork();
		if (right_pid == 0) {
			close(fds[1]);
			dup2(fds[0], STDIN_FILENO);
			close(fds[0]);
			exec_cmd(p->rightcmd);
			free_command((struct cmd *) p);
			_exit(1);
		}

		close(fds[0]);
		close(fds[1]);
		waitpid(left_pid, NULL, 0);
		waitpid(right_pid, &status, 0);
		break;
	}
}
