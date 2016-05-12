#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will pass-through its stdin without modifications.

expectedOutput=$(echo -e "A1\nA2\nA3")

assertCmdEq "$CLERR $HELPER/test-output.sh 2>/dev/null" "$expectedOutput"

success

