#ifndef __CLERR_H
#define __CLERR_H

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


#define VERSION "1.0.4"
#define VERDATE "May 2016"


int main (int argc, char** argv);


#define BUFFERSIZE 4096

#define EXIT_SYNTAXERR		1
#define EXIT_HELP		0
#define EXIT_INTERNALERR	125
#define EXIT_READERR		EXIT_INTERNALERR
#define EXIT_EXECERR		124

#define ANSI_RED     31
#define ANSI_GREEN   32
#define ANSI_YELLOW  33
#define ANSI_BLUE    34
#define ANSI_WHITE   37
#define ANSI_CYAN    36

#define DEFAULT_COLOR ANSI_RED

#define M1 "[1m"
#define M0 "[0m"


#endif // __CLERR_H
