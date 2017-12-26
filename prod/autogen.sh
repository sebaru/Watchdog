#! /bin/sh

svn update
autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
./configure
make -j 8
