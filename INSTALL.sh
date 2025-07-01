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
  dnf install -y awk git libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  dnf install -y glib2-devel openssl mosquitto-devel
  dnf install -y nut-devel libuuid-devel
  dnf install -y popt-devel libsoup3-devel gtts
  dnf install -y json-glib-devel gammu-devel
  dnf install -y wireplumber mpg123 sox libusb1-devel libgpiod-devel
  dnf install -y libstrophe-devel libphidget22-devel
  dnf install -y git systemd-devel libjwt-devel

  echo "/usr/local/lib" > /etc/ld.so.conf.d/local.conf
  echo "/usr/local/lib64" >> /etc/ld.so.conf.d/local.conf
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

  apt install -y awk git libtool automake autoconf gcc g++ git cmake openssl curl
  apt install -y libglib2.0-dev wireplumber mosquitto-dev
  apt install -y libupsclient-dev libssl-dev libstrophe-dev libgammu-dev
  apt install -y libpopt-dev libssl-dev libmariadb-dev libjwt-dev
  apt install -y sox libsox-fmt-all python3-pip mpg123
  apt install -y libjson-glib-dev libgpiod-dev
  apt install -y libsoup-3.0-dev alsa-utils libsystemd-dev
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
ldconfig

systemctl daemon-reload
systemctl enable --now Watchdogd
#      echo "Pour lancer Watchdog, tapez 'systemctl --user enable Watchdogd-user --now'"

  echo "Installation terminée. Rebootez puis linker l'agent."
  echo "Pour linker l'agent, utilisez Watchdogd --save --domain-uuid 'domain_uuid', --domain-secret 'domain_secret'"
  echo "Ou utiliser la console https://console.abls-habitat.fr/agent/add pour vous guider"
