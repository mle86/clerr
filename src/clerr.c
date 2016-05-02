#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include "clerr.h"

/*  Copyright (C) 2016  Maximilian L. Eul
    This file is part of clerr.

    clerr is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    clerr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with clerr.  If not, see <http://www.gnu.org/licenses/>.
*/


int fd_close;
pid_t pid_child = 0;

volatile int exit_status = 0;
volatile int exit_signal = 0;

static bool colorize_fd (int fd_read, short color, FILE* output);
static void run_command (int fd_write_errors, char** arguments);
static bool create_pipe (int *in, int *out);
static bool parse_color (char* arg, short *color);
static void sigchld     (int signo);
static void sigfwd      (int signo);
static void install_signal_handlers (void);
static void default_signal_handlers (void);
static void Version (void);
static void Help    (void);


int main (int argc, char** argv) {
	bool combined_output = false;
	short color = DEFAULT_COLOR;

	// Parse options
	{	signed char c;
		while ((c = getopt(argc, argv, "+hVc:1")) != -1)
		switch (c) {
			case 'h': Help(); return 0;
			case 'V': Version(); return 0;
			case '1': combined_output = true; break;
			case 'c':
				  if (! parse_color(optarg, &color)) {
					  fprintf(stderr, PROGNAME": unknown color\n");
					  return 0;
				  }
				  break;
		}
	}

	if ((argc - optind) < 1) {
		fprintf(stderr, PROGNAME": no command given\n");
		return 2;
	}

	if (color < 0 || color > 99) 
		return 9;

	int fd_write_errors, fd_read_errors;
	if (! create_pipe(&fd_write_errors, &fd_read_errors))
		return 1;

	// Install signal handler
	fd_close = fd_write_errors;
	install_signal_handlers();

	pid_child = fork();
	switch (pid_child) {
		case -1:;
			// fork() failed
			perror(PROGNAME": fork()");
			return 3;

		case 0:;
			// child process
			run_command(fd_write_errors, argv + optind);
			// run_command() is an execvp() wrapper, it returns if the exec call failed
			return 1;

		default:;
			// parent process -- read and print error messages from child:
			FILE* output = (combined_output) ? stdout : stderr;
			bool ret = colorize_fd(fd_read_errors, color, output);

			if (! ret)
				return 4;
			else if (exit_signal) {
				// kill self with same signal.
				default_signal_handlers();  // restore default signal handlers, or SIGINT won't work
				raise(exit_signal);
			} else
				// terminate with same status (might be zero)
				return exit_status;
	}
}


bool create_pipe (int *in, int *out) {
	int p [2];
	if (pipe(p) != 0) {
		perror(PROGNAME": pipe()");
		return false;
	}

	*in  = p[1];
	*out = p[0];

	return true;
}

bool colorize_fd (int fd_read, short color, FILE* output) {
	// prepare color sequences:
	const char a_reset [] = "[0m";
	char       a_color [] = "[1;YYm";
	snprintf(a_color, sizeof(a_color), "[1;%02him", color);

	const int fd_output = fileno(output);
	
	char buf [4096];

	while (true) {
		errno = 0;
		ssize_t s = read(fd_read, &buf, sizeof(buf) - 1);
		if (s > 0) {
			write(fd_output, a_color, sizeof(a_color) - 1);
			write(fd_output, buf, s);
			write(fd_output, a_reset, sizeof(a_reset) - 1);

			fflush(output);

		} else if (s == 0) {
			// EOF
			return true;

		} else if (errno == EINTR) {
			// retry
			
		} else {
			// Some other error condition.
			perror(PROGNAME": read()");
			return false;
		}
	}
}

void run_command (int fd_write_errors, char** arguments) {
	// save original stderr:
	const int fd_olderr = dup(fileno(stderr));

	// redirect stderr to fd_write_errors:
	dup2(fd_write_errors, fileno(stderr));

	/*      fd_olderr → real stderr
	 * child's stdout → real stdout
	 * child's stdin  ← real stdin
	 * child's stderr → fd_write_errors ⇒ fd_read_errors  */

	// restore default signal handlers:
	default_signal_handlers();

	// execute command:
	execvp(arguments[0], arguments);

	// couldn't execute? complain on original stderr:
	dup2(fd_olderr, fileno(stderr));
	perror(PROGNAME": execvp()");
}

void install_signal_handlers (void) {
	struct sigaction
		sa_sigchld = { .sa_handler = sigchld },
		sa_sigfwd  = { .sa_handler = sigfwd };

	sigemptyset(&sa_sigchld.sa_mask);
	sigemptyset(&sa_sigfwd.sa_mask);

	sigaction(SIGCHLD, &sa_sigchld, NULL);

	sigaction(SIGINT,  &sa_sigfwd,  NULL);
	sigaction(SIGTERM, &sa_sigfwd,  NULL);
	sigaction(SIGQUIT, &sa_sigfwd,  NULL);
	sigaction(SIGALRM, &sa_sigfwd,  NULL);
}

void default_signal_handlers (void) {
	struct sigaction sa_dfl = { .sa_handler = SIG_DFL };
	sigemptyset(&sa_dfl.sa_mask);

	sigaction(SIGCHLD, &sa_dfl, NULL);
	sigaction(SIGINT,  &sa_dfl, NULL);
	sigaction(SIGTERM, &sa_dfl, NULL);
	sigaction(SIGQUIT, &sa_dfl, NULL);
	sigaction(SIGALRM, &sa_dfl, NULL);
}

void sigchld (int signo) {
	/* We got a SIGCHLD signal.
	 * That means our stderr read() loop has now been interrupted with EINTR.
	 * This signal handler sets appropriate flags about how to continue.  */

	const int wait_options =
		WNOHANG;  // non-blocking, there already has been a SIGCHLD, so this call should always succeed

	int status;
	if (waitpid(pid_child, &status, wait_options) > 0) {
		if (WIFSTOPPED(status)) {
			/* The child process stopped. Wait for it to continue.
			 * We don't have to do anything special -- the current read() call was interrupted by this SIGCHLD,
			 * and the next read() (as soon as this signal handler returns) will simply block
			 * until something interesting happens.  */

		} else if (WIFCONTINUED(status)) {
			// The child process is running again.

		} else if (WIFEXITED(status)) {
			// The child process has ended. Make this process end too, with the same exit status.
			close(fd_close);  // make the next read() fail with EOF
			exit_status = WEXITSTATUS(status);

		} else if (WIFSIGNALED(status)) {
			// The child process has been killed. Kill this process too, with the same signal.
			close(fd_close);  // make the next read() fail with EOF
			exit_signal = WTERMSIG(status);
		}
	}

	(void)signo;  // suppress warning about unused parameter
}

void sigfwd (int signo) {
	/* We received a SIGINT or some other tty signal which should kill everything.
	 * This has interrupted our read() loop, but that's no problem.
	 * Without this signal handler, clerr would be killed off too early,
	 * possibly causing the child process' final stderr messages to be lost.
	 * So we just forward the signal to the child process and ignore it ourself.  */

	kill(pid_child, signo);
}


bool parse_color (char* arg, short *color) {
	if (!strcmp(arg, "gr") || !strcmp(arg, "gn") || !strcmp(arg, "green"))
		*color = ANSI_GREEN;
	else if (!strcmp(arg, "re") || !strcmp(arg, "rd") || !strcmp(arg, "red"))
		*color = ANSI_RED;
	else if (!strcmp(arg, "bl") || !strcmp(arg, "blue"))
		*color = ANSI_BLUE;
	else if (!strcmp(arg, "ye") || !strcmp(arg, "yw") || !strcmp(arg, "yellow"))
		*color = ANSI_YELLOW;
	else if (!strcmp(arg, "cy") || !strcmp(arg, "cn") || !strcmp(arg, "cyan"))
		*color = ANSI_CYAN;
	else if (!strcmp(arg, "wh") || !strcmp(arg, "white"))
		*color = ANSI_WHITE;
	else {
		// unknown color
		return false;
	}
	return true;
}


void Version () { printf(
	PROGNAME" v"VERSION"\n"
	"Written by Maximilian Eul <maximilian@eul.cc>, May 2016.\n"
	"\n"
); }


void Help () { printf(
	M1 PROGNAME M0" executes another program and colorizes all error output.\n"
	"Usage: "M1 PROGNAME M0" ["M1"OPTIONS"M0"] "M1"COMMAND"M0" ["M1"ARGUMENTS"M0"]\n"
	"Options:\n"
	"    "M1"-c COLOR"M0"  Select error color, choose from\n"
	"    "  "        "  "  white, red, green, blue, yellow, cyan.\n"
	"    "M1"-1      "M0"  Program output will be printed on stdout only.\n"
	"    "M1"-h      "M0"  This help.\n"
	"    "M1"-V      "M0"  Program version.\n"
	"Example:\n"
	"    "PROGNAME"  make -C ..\n"
	"\n"
); }

