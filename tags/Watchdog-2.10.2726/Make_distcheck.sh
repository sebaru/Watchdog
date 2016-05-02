#!/bin/sh
./update.sh
./autogen.sh
./configure
make distcheck
