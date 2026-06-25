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
  sudo dnf install -y git libtool cmake gcc pkg-config
  sudo dnf install -y glib2-devel openssl-devel json-glib-devel libuuid-devel
  sudo dnf install -y systemd-devel libjwt-devel mosquitto-devel
  sudo dnf install -y popt-devel libcurl-devel
  sudo dnf install -y gammu-devel libgpiod-devel nut-devel libstrophe-devel
  sudo dnf install -y libphidget22 libphidget22-devel

fi

if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ] || [ "$SOCLE" = "ubuntu" ]
 then
  echo "Installing debian/ubuntu dependencies"

  sudo apt update -y

  if [ "$SOCLE" = "raspbian" ]
   then
    sudo apt install -y gcc-8-base
  fi

  if [ "$SOCLE" = "ubuntu" ]
   then
    sudo apt install -y software-properties-common
    sudo add-apt-repository universe -y
    sudo apt update -y
  fi

  sudo apt install -y git libtool cmake gcc pkg-config
  sudo apt install -y libglib2.0-dev libssl-dev libjson-glib-dev uuid-dev
  sudo apt install -y libsystemd-dev libjwt-dev libmosquitto-dev
  sudo apt install -y libpopt-dev libcurl4-openssl-dev
  sudo apt install -y libgammu-dev libgpiod-dev libupsclient-dev libstrophe-dev
  curl -fsSL https://www.phidgets.com/downloads/setup_linux | bash -
  sudo apt install -y libphidget22 libphidget22-dev
fi
