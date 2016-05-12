#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will handle its arguments and return status correctly. 

# The first non-option argument should be interpreted as the command name:
assertCmdEq "$CLERR       $HELPER/args.sh  -c yellow  --  aa  bb" "<-c><yellow><--><aa><bb>"
# Of course, an explicit '--' option should not cause any problems either:
assertCmdEq "$CLERR   --  $HELPER/args.sh  -c yellow  --  aa  bb" "<-c><yellow><--><aa><bb>"
# We've only tested absolute paths so far. What about relative paths?
assertCmdEq "$CLERR ./helpers/args.sh yyy" "<yyy>"
# What about commands without a path?
assertCmdEq "$CLERR pwd" "$(pwd)"
# What about weird command arguments?
# (NB clerr invokes its command argument through execlp, so special characters need to be shell-escaped.)
assertCmdEq "$CLERR $HELPER/args.sh  aa \"'\" bb '\"' '<' cc" "<aa><'><bb><\"><<><cc>"

# All commands so far have been expected to work.
# What about broken commands?
assertCmd "$CLERR $HELPER/exit.sh 35" 35
assertCmd "$CLERR $HELPER/exit.sh 2"  2
# Or self-killing commands?
assertCmd "$CLERR $HELPER/selfkill.sh TERM" $((128 + 15))
assertCmd "$CLERR $HELPER/selfkill.sh KILL" $((128 + 9))
# Or missing commands?
assertCmd "$CLERR $HELPER/DOES-NOT-EXIST.sh" 1
# Or existing, but non-executable commands?
assertCmd "$CLERR $HELPER/noexec.sh" 1

success

