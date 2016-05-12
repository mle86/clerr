#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests clerr's -1 option.

expectedOutput=$(echo -ne \
"A1\n"\
"${clerr_color_error}E1\n${clerr_color_reset}"\
"A2\n"\
"${clerr_color_error}E2\n${clerr_color_reset}"\
"A3\n"\
"${clerr_color_error}E3\n${clerr_color_reset}" )

assertCmdEq "$CLERR -1 $HELPER/test-output.sh" "$expectedOutput"

success

