#!/bin/sh

SOCLE=`grep "^ID" /etc/os-release | cut -f 2 -d '='`
USER=`whoami`

echo "faut-il installer le SGBD local ? (oui/non)"
read -p "install_sgbd: " sgbd

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
  dnf install -y libcurl-devel nut-devel mariadb-devel zeromq-devel libuuid-devel
  dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  dnf install -y gtksourceview2-devel goocanvas-devel json-glib-devel gammu-devel
  dnf install -y vlc alsa-utils alsa-firmware
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

svn co https://svn.abls-habitat.fr/repo/Watchdog/prod watchdogabls
cd watchdogabls
echo "Compiling and installing"
./autogen.sh
make install
cd ..
rm -rf watchdogabls
systemctl daemon-reload

CONFFILE="/etc/watchdogd.abls.onf"
if [ ! -f $CONFFILE ]
 then
    cp "/usr/local/etc/watchdogd.abls.conf.sample" $CONFFILE
fi


 targetdir="/var/www/html/WEB"
 mkdir -p $targetdir
 if [ "$SOCLE" = "fedora" ]
  then dnf -y install httpd php-json php php-mysqlnd
  chown apache.apache -R $targetdir
  sudo -u apache svn co https://svn.abls-habitat.fr/repo/Watchdog/prod/Interface_API $targetdir
  cp watchdogd-httpd.conf /etc/httpd/conf.d/
  /sbin/restorecon -rv $targetdir
  systemctl restart httpd
 fi
 if [ "$SOCLE" = "debian" ]
  then apt -y install apache2 php php7.3-mysql php-curl
  a2enmod proxy
  a2enmod proxy_wstunnel
  a2enmod headers
  a2enmod rewrite
  a2dissite 000-default
  chown www-data $targetdir
  sudo -u www-data svn co https://svn.abls-habitat.fr/repo/Watchdog/prod/Interface_API $targetdir
  cp Interface_API/watchdogd-httpd.conf /etc/apache2/sites-available/
  a2ensite watchdogd-httpd
  systemctl reload apache2
 fi
# if [ ! -f "$targetdir/application/config/config.php" ]
#  then
#   cp Interface_WEB/application/config/config.php.sample $targetdir/application/config/config.php
# fi

if [ "$sgbd" = "oui" ]
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
    echo "La base de données 'WatchdogDB' a été crée, ainsi que l'utilisateur 'watchdog'."
    echo "Son mot de passe est "$NEWPASSWORD
    echo "Le point d'accès pour poursuivre l'installation est http://"`hostname`":5560/install"
fi

systemctl restart Watchdogd
