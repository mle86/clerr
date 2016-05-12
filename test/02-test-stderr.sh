#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will correctly colorize its error input.

expectedOutput=$(echo -ne \
"${clerr_color_error}E1\n${clerr_color_reset}"\
"${clerr_color_error}E2\n${clerr_color_reset}"\
"${clerr_color_error}E3\n${clerr_color_reset}" )

assertCmdEq "$CLERR $HELPER/test-output.sh 2>&1 >/dev/null" "$expectedOutput"

success

