#!/bin/bash -e

if [ "$1" == "--debug" ]; then
  shift
  LD_LIBRARY_PATH=build/src:build/asdcplib/src gdb --args build/test/tests
else
  LD_LIBRARY_PATH=build/src:build/asdcplib/src build/test/tests
fi
