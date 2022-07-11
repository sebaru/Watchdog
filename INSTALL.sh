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
  dnf update -y
  dnf install -y libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  dnf install -y glib2-devel bison flex giflib-devel openssl
  dnf install -y nut-devel mariadb-devel libuuid-devel
  dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  dnf install -y json-glib-devel gammu-devel
  dnf install -y mpg123 sox libusb-devel libgpiod-devel
  dnf install -y librsvg2-devel
  dnf install -y git systemd-devel libjwt-devel

  git clone https://github.com/strophe/libstrophe.git
  cd libstrophe
  ./bootstrap.sh
  ./configure
  make
  make install
  cd ..
  rm -rf libstrophe

  curl -fsSL https://www.phidgets.com/downloads/phidget22/libraries/linux/libphidget22.tar.gz | tar xvz
  cd libphidget22-*
  ./configure
  make
  make install
  cd ..
  rm -rf libphidget22*

  echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf
  ldconfig

fi


if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ]
 then
  echo "Installing debian dependencies"
  apt update
  apt upgrade -y

  if [ "$SOCLE" = "raspbian" ]
   then
    apt install -y gcc-8-base
  fi

  apt install -y subversion libtool automake autoconf gcc git cmake openssl
  apt install -y libglib2.0-dev bison flex libgif-dev
  apt install -y libupsclient-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
  apt install -y libpopt-dev libssl-dev libmariadbclient-dev
  apt install -y sox libsox-fmt-all python3-pip mpg123
  apt install -y libjson-glib-dev libgpiod-dev
  apt install -y libgtk-3-dev libgoocanvas-2.0-dev
  apt install -y libsoup2.4-dev librsvg2-dev alsa-utils libsystemd-dev
  pip3 install gTTS-token gTTS --upgrade
  curl -fsSL https://www.phidgets.com/downloads/setup_linux | bash -
  apt install -y libphidget22 libphidget22-dev
fi

git clone https://github.com/sebaru/Watchdog.git watchdogabls
cd watchdogabls
echo "Compiling and installing"
./autogen.sh
make install
cd ..
rm -rf watchdogabls
systemctl daemon-reload
systemctl start Watchdogd
#      echo "Pour lancer Watchdog, tapez 'systemctl --user enable Watchdogd-user --now'"

  echo "Le point d'accès pour poursuivre l'installation est https://localhost:5559/install"
