#!/bin/sh

SOCLE = `grep "^ID" /etc/os-release | cut -f 2 -d '='`
USER  =`whoami`

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf update
  dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
  dnf install -y libcurl-devel nut-devel mariadb-devel zeromq-devel
  dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  dnf install -y gtksourceview2-devel goocanvas-devel json-glib-devel gammu-devel
  dnf install -y vlc alsa-utils alsa-firmware
  dnf install -y mpg123 sox mosquitto-devel
fi

if [ "$SOCLE" = "debian" ]
 then
  echo "Installing debian dependencies"
  apt update
  apt install -y subversion libtool automake autoconf gcc git cmake
  apt install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt20-dev
  apt install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
  apt install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev
  apt install -y sox libsox-fmt-all python3-pip mpg123
  apt install -y libjson-glib-dev libmosquitto-dev vlc
  apt install -y libgtk-3-dev libsoup2.4-dev libgoocanvas-2.0-dev
fi

echo "L'instance est-elle master ? (oui/non)"
read -p "instance_is_master: " master

if [ "$master" = "oui" ]
	then
		sudo cp /usr/local/etc/Watchdogd.service.master /etc/systemd/system/Watchdogd.service
	else
  sudo cp /usr/local/etc/Watchdogd.service.slave /etc/systemd/system/Watchdogd.service
fi
sudo systemctl daemon-reload
sudo usermod -a -G audio,dialout $USER

echo "Copying data files for $USER"
mkdir -p ~/Son
mkdir -p ~/Dls
cp -r Son/* ~/Son
echo "done."
sleep 2

echo "Compiling and installing"
./autogen.sh
sudo make install

CONFFILE="/etc/watchdogd.conf"
if [ ! -f $CONFFILE ]
 then
    cp "/usr/local/etc/watchdogd.conf.sample" $CONFFILE
fi

sed -i $CONFFILE -e "s#usertobechanged#$USER#g"

echo "faut-il installer le SGBD ? (oui/non)"
read -p "install_sgbd: " sgbd

if [ "$sgbd" = "oui" ]
 then
    if [ "$SOCLE" = "fedora" ]
     then dnf -y install mariadb-server
    fi
    if [ "$SOCLE" = "debian" ]
     then apt -y install mariadb-server
    fi
    sudo systemctl restart mariadb

    NEWPASSWORD=`openssl rand -base64 32`
    sed $CONFFILE "s#dbpasstobechanged#$NEWPASSWORD#g"
    /usr/bin/mysqladmin -u root create WatchdogDB
     echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
    /usr/bin/mysql_secure_installation
fi
