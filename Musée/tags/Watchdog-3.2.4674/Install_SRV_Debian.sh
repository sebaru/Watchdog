#!/bin/sh

echo "Installing dependencies"
apt-get update
apt-get install -y subversion libtool automake autoconf gcc git cmake
apt-get install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt20-dev
apt-get install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
apt-get install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev libwebsockets-dev
apt-get install -y sox libsox-fmt-all python3-pip mpg123
apt-get install -y libxml2-dev libjson-glib-dev libmosquitto-dev vlc 
pip3 install gTTS
