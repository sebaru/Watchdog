#! /bin/sh

svn update
osFedora=`grep "NAME=Fedora" /etc/os-release`
osRaspbian=`grep "ID=raspbian" /etc/os-release`

if [ -n "$osFedora" ]; then
  echo "Compiling for Fedora !"
  cp Makefile.am.fedora Makefile.am
  cp Watchdogd/Makefile.am.fedora Watchdogd/Makefile.am

fi

if [ -n "$osRaspbian" ]; then
  echo "Compiling for Raspbian !"
  cp Makefile.am.raspbian Makefile.am
  cp Watchdogd/Makefile.am.raspbian Watchdogd/Makefile.am
fi

autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
./configure
make -j 8
