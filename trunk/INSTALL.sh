#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
   echo "Only user root can run this script (or sudo)."
   exit 1
fi

read -p "Install for (S)ystemMode or (U)serMode (s/u) ?" -n1 USERMODE
echo

if [ "$USERMODE" = "u" ]
 then
   USERMODE="Y"; echo "Installing in user mode in 4 seconds";
 else
   USERMODE="N"; echo "Installing in system mode in 4 seconds";
fi

sleep 4

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf update -y
  dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  dnf install -y glib2-devel bison flex giflib-devel
  dnf install -y nut-devel mariadb-devel zeromq-devel libuuid-devel
  dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  dnf install -y json-glib-devel gammu-devel
  dnf install -y mpg123 sox libusb-devel
  dnf install -y librsvg2-devel
  dnf install -y git

  git clone https://github.com/strophe/libstrophe.git
  cd libstrophe
  ./bootstrap.sh
  ./configure
  make
  make install
  cd ..
  rm -rf libstrophe

  wget https://www.phidgets.com/downloads/phidget22/libraries/linux/libphidget22.tar.gz
  tar xzf libphidget22.tar.gz
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

  apt install -y subversion libtool automake autoconf gcc git cmake
  apt install -y libglib2.0-dev bison flex libgif-dev
  apt install -y libupsclient-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
  apt install -y libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev
  apt install -y sox libsox-fmt-all python3-pip mpg123
  apt install -y libjson-glib-dev
  apt install -y libgtk-3-dev libgoocanvas-2.0-dev
  apt install -y libsoup2.4-dev librsvg2-dev
  curl -fsSL https://www.phidgets.com/downloads/setup_linux | bash -
  apt install -y libphidget22 libphidget22-dev
fi

  if [ "$USERMODE" = "N" ]
   then
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
  fi

svn co https://svn.abls-habitat.fr/repo/Watchdog/prod watchdogabls
cd watchdogabls
echo "Compiling and installing"
./autogen.sh
make install
cd ..
rm -rf watchdogabls
systemctl daemon-reload

  if [ "$USERMODE" = "N" ]
    then
      echo "La base de données 'WatchdogDB' a été crée, ainsi que l'utilisateur 'watchdog'."
      echo "Son mot de passe est "$NEWPASSWORD
      systemctl enable Watchdogd --now
    else
      echo "Pour lancer Watchdog, tapez 'systemctl --user enable Watchdogd --now'"
  fi

  echo "Le point d'accès pour poursuivre l'installation est https://localhost:5560/install"
