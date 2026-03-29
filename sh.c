#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

char prompt[PRMTLEN] = { 0 };
void sigchld_handler(int);
void configurar_sigchld(void);

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

void
sigchld_handler(int)
{
	pid_t pid;
	int status;
	int saved_errno = errno;

	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		if (WIFEXITED(status)) {
			fprintf_debug(stderr,
			              "==> terminado: PID=%d (exit=%d)\n",
			              pid,
			              WEXITSTATUS(status));
		} else if (WIFSIGNALED(status)) {
			fprintf_debug(
			        stderr,
			        "==> terminado: PID=%d (killed by signal %d)\n",
			        pid,
			        WTERMSIG(status));
		} else {
			fprintf_debug(
			        stderr,
			        "==> terminado: PID=%d (unknown status)\n",
			        pid);
		}
	}

	errno = saved_errno;
}

void
configurar_sigchld()
{
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		printf_debug("Error en sigaction.\n");
		exit(1);
	}
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	configurar_sigchld();

	run_shell();

	return 0;
}
