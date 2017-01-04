#!/bin/sh

echo "Installing dependencies"
apt-get install -y subversion libtool automake autoconf gcc
apt-get install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt11-dev librrd-dev
apt-get install -y libupsclient1-dev libcurl4-gnutls-dev libssl-dev libmysqlclient-dev libgnokii-dev
apt-get install -y liblircclient-dev libloudmouth1-dev libmicrohttpd-dev libpopt-dev
apt-get install -y libgnomeui-dev libgoocanvas-dev libgtksourceview2.0-dev 

#echo "Creating systemd service"
#cp Watchdogd.service /lib/systemd/system/
#systemctl enable Watchdogd.service
echo "done."
