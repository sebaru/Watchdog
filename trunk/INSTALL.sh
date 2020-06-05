#!/bin/sh

SOCLE=`grep "^ID" /etc/os-release | cut -f 2 -d '='`
USER=`whoami`

if [ "$SOCLE" = "fedora" ]
 then
  echo "Installing Fedora dependencies"
  sudo dnf install -y subversion libtool automake autoconf gcc gcc-c++ redhat-rpm-config
  sudo dnf install -y glib2-devel bison flex readline-devel giflib-devel libgcrypt-devel
  sudo dnf install -y libcurl-devel nut-devel mariadb-devel zeromq-devel libuuid-devel
  sudo dnf install -y gtk3-devel goocanvas2-devel popt-devel libsoup-devel
  sudo dnf install -y gtksourceview2-devel goocanvas-devel json-glib-devel gammu-devel
  sudo dnf install -y vlc alsa-utils alsa-firmware
  sudo dnf install -y mpg123 sox mosquitto-devel
  sudo dnf install -y libgnomeui-devel 
  sudo dnf install -y git
  git clone https://github.com/strophe/libstrophe.git
  cd libstrophe
  ./bootstrap.sh
  ./configure
  make
  sudo make install
  cd ..
fi

if [ "$SOCLE" = "debian" ]
 then
  echo "Installing debian dependencies"
  sudo apt update
  sudo apt install -y subversion libtool automake autoconf gcc git cmake
  sudo apt install -y libglib2.0-dev bison flex libreadline-dev libgif-dev libgcrypt20-dev
  sudo apt install -y libupsclient-dev libcurl4-gnutls-dev libssl-dev default-libmysqlclient-dev libstrophe-dev libgammu-dev
  sudo apt install -y liblircclient-dev libpopt-dev libssl-dev libmariadbclient-dev libzmq3-dev
  sudo apt install -y sox libsox-fmt-all python3-pip mpg123
  sudo apt install -y libjson-glib-dev libmosquitto-dev vlc
  sudo apt install -y libgtk-3-dev libsoup2.4-dev libgoocanvas-2.0-dev
fi

echo "Copying data files for $USER"
mkdir -p ~/Son
mkdir -p ~/Dls
cp -r Son/* ~/Son
echo "done."
sleep 2

echo "Compiling and installing"
./autogen.sh
sudo make install

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

CONFFILE="/etc/watchdogd.conf"
if [ ! -f $CONFFILE ]
 then
    sudo cp "/usr/local/etc/watchdogd.conf.sample" $CONFFILE
fi

sudo sed -i $CONFFILE -e "s#usertobechanged#$USER#g"

echo "faut-il installer le SGBD ? (oui/non)"
read -p "install_sgbd: " sgbd

if [ "$sgbd" = "oui" ]
 then
    if [ "$SOCLE" = "fedora" ]
     then sudo dnf -y install mariadb-server
    fi
    if [ "$SOCLE" = "debian" ]
     then sudo apt -y install mariadb-server
	   sudo mysql_install_db
    fi
    sudo systemctl restart mariadb

    NEWPASSWORD=`openssl rand -base64 32`
    sudo sed -i $CONFFILE -e "s#dbpasstobechanged#$NEWPASSWORD#g"
    /usr/bin/mysqladmin -u root create WatchdogDB
     echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
    /usr/bin/mysql_secure_installation
fi

echo "faut-il installer l'interface WEB ? (oui/non)"
read -p "install_web: " web

if [ "$web" = "oui" ]
 then
    targetdir="/var/www/html/WEB"
    sudo mkdir -p $targetdir
    if [ "$SOCLE" = "fedora" ]
     then sudo dnf -y install httpd php-json php php-mysqlnd
     sudo chown apache.apache -R $targetdir
     sudo -u apache svn co https://svn.abls-habitat.fr/repo/Watchdog/trunk/Interface_WEB $targetdir
     sudo cp Interface_WEB/watchdogd-httpd.conf /etc/httpd/conf.d/
     sudo /sbin/restorecon -rv $targetdir
     sudo systemctl restart httpd
    fi
    if [ "$SOCLE" = "debian" ]
     then sudo apt -y install apache2 php php7.3-mysql php-curl
     sudo a2enmod proxy
     sudo a2enmod proxy_wstunnel
     sudo a2enmod headers
     sudo a2enmod rewrite
     sudo a2dissite 000-default
     sudo chown www-data $targetdir
     sudo -u www-data svn co https://svn.abls-habitat.fr/repo/Watchdog/trunk/Interface_WEB $targetdir
     sudo cp Interface_WEB/watchdogd-httpd.conf /etc/apache2/sites-available/
     sudo a2ensite watchdogd-httpd
     sudo systemctl reload apache2
    fi
    if [ ! -f "$targetdir/application/config/config.php" ]
     then
	     sudo cp Interface_WEB/application/config/config.php.sample $targetdir/application/config/config.php
    fi
    if [ ! -f "$targetdir/application/config/database.php" ]
     then
	     sudo cp Interface_WEB/application/config/database.php.sample $targetdir/application/config/database.php
	     sudo sed -i $targetdir/application/config/database.php -e "s#dbpasstobechanged#$NEWPASSWORD#g"
    fi
    fi

if [ "$SOCLE" = "fedora" ]
 then
  echo "Executing dnf auto-remove"
  sudo dnf autoremove
fi

if [ "$SOCLE" = "debian" ]
 then
  echo "Executing apt auto-remove"
  sudo apt autoremove
fi

sudo systemctl restart Watchdogd
