#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will work correctly if its command gets STOPped.

timeout=0.4

expectedOutput=$(echo -ne \
"A1\n"\
"${clerr_color_error}E1\n${clerr_color_reset}"\
"A2\n"\
"${clerr_color_error}E2\n${clerr_color_reset}" )

# Launch stopper.sh, it'll print A1\nE1 and SIGSTOP itself.
# Then after a short delay, timeout will SIGCONT it.
# selfkill.sh should then continue and hit its "exit 95" line.
assertCmd "$CLERR -1  timeout --preserve-status --signal=CONT $timeout  $HELPER/stopper.sh" 86
assertEq "$ASSERTCMDOUTPUT" "$expectedOutput"

# The same thing, but this time, the timeout tool will SIGCONT clerr, not its command.
# This should work too, because timeout (without --foreground) signals its command's whole process group.
assertCmd "timeout --preserve-status --signal=CONT $timeout  $CLERR -1  $HELPER/stopper.sh" 86
assertEq "$ASSERTCMDOUTPUT" "$expectedOutput"

success

