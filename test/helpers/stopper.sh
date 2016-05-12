#!/bin/sh

sleep="${1:-.001}"
delay () {
	[ -n "$sleep" ] && [ "$sleep" != "0" ] && sleep "$sleep"
}

echo "A1"
delay
echo "E1" >&2

kill -s STOP $$

echo "A2"
delay
echo "E2" >&2

exit 86

