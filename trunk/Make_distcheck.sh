#!/bin/sh
svn update
svnversion -n | sed -e "s/M//g" > RevisionNumber.txt
./autogen.sh
make -j 2 distcheck
