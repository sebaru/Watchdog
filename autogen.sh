#! /bin/sh

git pull
echo "We're on branch "$(git branch --show-current)
echo "Compiling "$(git describe --tags | tr -d '\n')
osFedora=`grep -e "^NAME.*Fedora.*" /etc/os-release`
osRaspbian=`grep "ID=raspbian" /etc/os-release`
osDebian=`grep "ID=debian" /etc/os-release`

if [ -n "$osFedora" ]; then
  echo "Compiling for Fedora !"
  cp Watchdogd/Makefile.fedora.am Watchdogd/Makefile.am

fi

if [ -n "$osRaspbian" ]; then
  echo "Compiling for Raspbian !"
  cp Watchdogd/Makefile.raspbian.am Watchdogd/Makefile.am
fi

if [ -n "$osDebian" ]; then
  echo "Compiling for Debian !"
  cp Watchdogd/Makefile.raspbian.am Watchdogd/Makefile.am
fi

autoheader
libtoolize
aclocal --force
automake --add-missing
autoconf --force
./configure
make -j `grep -c ^processor /proc/cpuinfo`
