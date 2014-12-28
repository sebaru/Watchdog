#!/bin/sh
./update.sh
./autogen.sh
make -j 2 distcheck
