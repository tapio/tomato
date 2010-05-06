#!/bin/sh
# Script name: nettest.sh
# Description: Script to easily test the network game on a single computer.
#              Designed to be run from the build directory.
# Version:     1

make install

Tomaatti --server &
Tomaatti --client &
Tomaatti --client

killall Tomaatti
