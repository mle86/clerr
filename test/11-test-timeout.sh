#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will work correctly if its command suddenly terminates due to a signal.

timeout=0.6
msgdelay=.18
expectedOutput=$(echo -e "A1\nA2")

assertCmd "$CLERR  timeout --preserve-status $timeout  $HELPER/test-output.sh $msgdelay 2>/dev/null" $((128 + 15))
assertEq "$ASSERTCMDOUTPUT" "$expectedOutput"

success

