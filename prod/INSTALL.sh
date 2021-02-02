#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`
USER=`whoami`


if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf update -y
  dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config openssl
  dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
  dnf install -y libcurl-devel nut-devel mariadb-devel zeromq-devel libuuid-devel
  dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  dnf install -y gtksourceview2-devel goocanvas-devel json-glib-devel gammu-devel
  dnf install -y mpg123 sox mosquitto-devel
  dnf install -y libgnomeui-devel
  dnf install -y git
  git clone https://github.com/strophe/libstrophe.git
  cd libstrophe
  ./bootstrap.sh
  ./configure
  make
  make install
  cd ..
  rm -rf libstrophe
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
  apt install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt20-dev
  apt install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
  apt install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev
  apt install -y sox libsox-fmt-all python3-pip mpg123
  apt install -y libjson-glib-dev libmosquitto-dev
  apt install -y libgtk-3-dev libgoocanvas-2.0-dev
  apt install -y libsoup2.4-dev
fi

    if [ "$SOCLE" = "fedora" ]
     then dnf -y install mariadb-server
    fi
    if [ "$SOCLE" = "debian" ]
     then apt -y install mariadb-server
	   mysql_install_db
    fi
    systemctl restart mariadb

    NEWPASSWORD=`openssl rand -base64 32`
    /usr/bin/mysqladmin -u root create WatchdogDB
    echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; " | mysql -u root WatchdogDB

svn co https://svn.abls-habitat.fr/repo/Watchdog/prod watchdogabls
cd watchdogabls
echo "Compiling and installing"
./autogen.sh
make install
cd ..
rm -rf watchdogabls
systemctl daemon-reload

    echo "La base de données 'WatchdogDB' a été crée, ainsi que l'utilisateur 'watchdog'."
    echo "Son mot de passe est "$NEWPASSWORD
    echo "Le point d'accès pour poursuivre l'installation est https://"`hostname`":5560/install"

systemctl enable Watchdogd --now
