#!/bin/sh

sleep="${1:-.001}"
delay () {
	[ -n "$sleep" ] && [ "$sleep" != "0" ] && sleep "$sleep"
}

echo "A1"
delay
echo "E1" >&2
delay

echo "A2"
delay
echo "E2" >&2
delay

echo "A3"
delay
echo "E3" >&2

