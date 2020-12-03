#!/bin/sh
ROOT_DIR=$(pwd)
SCRIPT_PATH=scripts/sstar/gdb
arm-linux-gnueabihf-gdb -command=$ROOT_DIR/$SCRIPT_PATH/main.gdb
