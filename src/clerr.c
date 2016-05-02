#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include "clerr.h"

/*  Copyright (C) 2013  Maximilian L. Eul
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
int ret = 0;


int main (int argc, char** argv) {
	char buf [4096];
	ssize_t s = 0;
	
	int cnt;

	bool SameOutput = false;
	short color = DEFAULT_COLOR;

	int f[2], fd_child_errors, fd_errors, fd_olderr;


	// Create pipe for error messages from child
	if (pipe(f)) {
		perror(PROGNAME": pipe()");
		return 1;
	}
	fd_child_errors = f[1 /*pipe in*/];
	fd_errors       = f[0 /*pipe out*/];

	// Parse options
	{	register signed char c;
		while( (c = getopt(argc, argv, "+hVc:1")) != -1 )
		switch (c) {
			case 'h': Help(); return 0;
			case 'V': Version(); return 0;
			case '1': SameOutput = true; break;
			case 'c': setColor(optarg, &color); break;
		}
	}

	cnt = argc - optind;
	if (cnt < 1) {
		fprintf(stderr, PROGNAME": no command given\n");
		return 2;
	}

	if (color < 0 || color > 99) 
		return 9;

	// Install signal handler
	fd_close = fd_child_errors;
	signal(SIGCHLD, sig);

	{
		// Prepare output format string
		char fmt [] = "[1;YYm";
		snprintf(fmt, sizeof(fmt), "[1;%02him", color);

		// Prepare output stream
		FILE* const out = SameOutput ? stdout : stderr;
		const int outfd = fileno(out);

		switch(fork()) {

			case -1:
				perror(PROGNAME": fork()");
				return 3;

			case 0:
				fd_olderr = dup(fileno(stderr));
				dup2(fd_child_errors, fileno(stderr));

				//      fd_olderr --> real stderr (console)
				// child's stdout --> real stdout (console)
				// child's stdin  <-- real stdin  (console)
				// child's stderr --> fd_child_errors ==> fd_errors

				// Signal handler not needed in this thread
				signal(SIGCHLD, SIG_DFL);

				// Execute program
				execvp(argv[optind], argv+optind);

				// Couldn't execute? Complain on normal stderr
				dup2(fd_olderr, fileno(stderr));
				perror(PROGNAME": execvp()");
				return 1;

			default:
				// Read and print error messages from child
				while( (s = read(fd_errors, &buf, sizeof(buf)-1)) > 0 ) {

					write(outfd, fmt, sizeof(fmt)-1);
					write(outfd, buf, s);
					write(outfd, "[0m", 4);

					fflush(out);
				}

				if (s) {
					// read error
					perror(PROGNAME": read()");
					return 4;
				}
				break;
		}
	}

	// EOF
	// (or some error within child process which caused no read error)
	return ret;
}


void sig (int s) {
	int status;

	close(fd_close);

	if (waitpid(0, &status, WNOHANG) > 0)
		ret = WEXITSTATUS(status);

	// The signal number is irrelevant.
	// Prevent "unused parameter" warning:
	(void)s;
}


void setColor (char* arg, short *color) {
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
}


void Version () { printf(
	PROGNAME" v"VERSION"\n"
	"Written by Maximilian Eul <maximilian@eul.cc>, June 2013.\n"
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
