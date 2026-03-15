#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
   echo "Only user root can run this script (or sudo)."
   exit 1
fi

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf install -y git libtool cmake gcc pkg-config
  dnf install -y glib2-devel openssl-devel json-glib-devel libuuid-devel
  dnf install -y systemd-devel libjwt-devel mosquitto-devel
  dnf install -y popt-devel libcurl-devel
  dnf install -y gammu-devel libgpiod-devel nut-devel libstrophe-devel
  curl -fsSL https://www.phidgets.com/downloads/setup_linux | bash -
  dnf install -y libphidget22 libphidget22-devel

fi

if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ] || [ "$SOCLE" = "ubuntu" ]
 then
  echo "Installing debian/ubuntu dependencies"

  apt update -y

  if [ "$SOCLE" = "raspbian" ]
   then
    apt install -y gcc-8-base
  fi

  if [ "$SOCLE" = "ubuntu" ]
   then
    apt install -y software-properties-common
    add-apt-repository universe -y
    apt update -y
  fi

  apt install -y git libtool cmake gcc pkg-config
  apt install -y libglib2.0-dev libssl-dev libjson-glib-dev uuid-dev
  apt install -y libsystemd-dev libjwt-dev libmosquitto-dev
  apt install -y libpopt-dev libcurl4-openssl-dev
  apt install -y libgammu-dev libgpiod-dev libupsclient-dev libstrophe-dev
  curl -fsSL https://www.phidgets.com/downloads/setup_linux | bash -
  apt install -y libphidget22 libphidget22-dev
fi
