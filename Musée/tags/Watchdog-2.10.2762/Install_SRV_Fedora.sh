#!/bin/sh

echo "Installing dependencies"
dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config
dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel rrdtool-devel
dnf install -y libcurl-devel nut-devel mysql-devel gnokii-devel libmicrohttpd-devel 
dnf install -y lirc-devel loudmouth-devel libxml2-devel libgnomeui-devel popt-devel 
dnf install -y gtksourceview2-devel goocanvas-devel gtkdatabox-devel libgnomeui-devel  

echo "Creating systemd service"
cp Watchdogd.service /lib/systemd/system/
systemctl enable Watchdogd.service
echo "done."
