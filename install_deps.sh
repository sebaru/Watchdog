#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
   echo "Only user root can run this script (or sudo)."
   exit 1
fi

groupadd abls

if [ "$SOCLE" = "fedora" ]
 then
  echo "Configuring ABLS-PKGS repository"
  curl -fsSL https://pkgs.abls-habitat.fr/abls-rpms.repo -o /etc/yum.repos.d/abls-rpms.repo

  echo "Installing RPM-based dependencies"
  dnf install -y abls-libs-devel git libtool cmake gcc pkg-config
  dnf install -y glib2-devel openssl-devel json-glib-devel libuuid-devel
  dnf install -y systemd-devel libjwt-devel mosquitto-devel
  dnf install -y popt-devel libcurl-devel
  dnf install -y gammu-devel libgpiod-devel nut-devel libstrophe-devel
  dnf install -y libphidget22 libphidget22-devel

fi

if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ] || [ "$SOCLE" = "ubuntu" ]
 then
  echo "Configuring ABLS APT repository"
  curl -fsSL https://pkgs.abls-habitat.fr/rpms/keys/RPM-GPG-KEY-ABLS | gpg --dearmor -o /usr/share/keyrings/abls-archive-keyring.gpg
  curl -fsSL https://pkgs.abls-habitat.fr/abls-deb.sources -o /etc/apt/sources.list.d/abls-pkgs.sources

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
