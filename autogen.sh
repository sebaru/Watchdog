#! /bin/sh

autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
