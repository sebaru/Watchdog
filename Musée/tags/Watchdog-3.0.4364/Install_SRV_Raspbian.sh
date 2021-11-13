#!/bin/sh

echo "Installing dependencies"
apt-get update
apt-get install -y subversion libtool automake autoconf gcc git cmake
apt-get install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt11-dev
apt-get install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libpurple-dev libgammu-dev
apt-get install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev
apt-get install -y sox libsox-fmt-all python3-pip mpg123
apt-get install -y libgnomeui-dev libgoocanvas-dev libgtksourceview2.0-dev libjson-glib-dev libmosquitto-dev vlc 
pip3 install gTTS

git clone https://github.com/warmcat/libwebsockets.git
cd libwebsockets
cmake .
make -j 2
make install
