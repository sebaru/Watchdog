#!/bin/sh

echo "Installing dependencies"
dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config libwebsockets-devel
dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
dnf install -y libcurl-devel nut-devel mysql-devel gnokii-devel 
dnf install -y lirc-devel loudmouth-devel libxml2-devel libgnomeui-devel popt-devel 
dnf install -y gtksourceview2-devel goocanvas-devel libgnomeui-devel  json-glib-devel
dnf install -y pulseaudio alsa-utils alsa-firmware mpg123
dnf install -y sox sox-plugins-freeworld
pip3 install google_speech

