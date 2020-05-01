#!/bin/sh

echo "Installing dependencies"
apt update
apt install -y subversion libtool automake autoconf gcc git cmake
apt install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt20-dev
apt install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
apt install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev libwebsockets-dev
apt install -y sox libsox-fmt-all python3-pip mpg123
apt install -y libxml2-dev libjson-glib-dev libmosquitto-dev vlc 
apt install -y libgtk-3-dev libsoup2.4-dev
pip3 install gTTS
