#!/bin/bash
PWD=`pwd`

mkdir -p build
pushd build
cmake ..
make
popd

