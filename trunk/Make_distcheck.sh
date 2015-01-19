#!/bin/sh
./update.sh
./autogen.sh
./configure
make -j 2 distcheck
