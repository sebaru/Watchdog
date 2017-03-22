#! /bin/sh

svn update
autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
./configure
time make -j 4
