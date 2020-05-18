#!/bin/sh

echo "Installing dependencies"
dnf install -y subversion mariadb-server libtool automake autoconf gcc gcc-c++ redhat-rpm-config
dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
dnf install -y libcurl-devel nut-devel mariadb-devel zeromq-devel
dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
dnf install -y gtksourceview2-devel goocanvas-devel json-glib-devel gammu-devel
dnf install -y vlc alsa-utils alsa-firmware 
dnf install -y mpg123 sox mosquitto-devel
pip3 install gTTS
