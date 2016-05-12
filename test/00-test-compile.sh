#!/bin/bash
. $(dirname "$0")/init.sh

# This script tests if clerr will compile.

assertCmd -v "make -C $HERE/../"

success

