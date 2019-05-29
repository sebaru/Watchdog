#!/bin/sh

echo "Creating systemd service"
wtd_home=/var/lib/watchdog

echo "Quel user fera tourner Watchdog ?"
read wtd_user

echo "Dois-je installé la Database (oui/non) ?"
read database

echo "Installation in standalone mode in $wtd_home for $wtd_user"
sleep 5
sudo cp /usr/local/etc/Watchdogd.service.system /etc/systemd/system/Watchdogd.service
sudo systemctl daemon-reload
sudo systemctl enable Watchdogd.service
sudo usermod -a -G audio,dialout $wtd_user
sudo loginctl enable-linger $wtd_user
sudo systemctl daemon-reload
echo "done."
sleep 2

echo "Copying data files"
sudo mkdir -p $wtd_home
sudo chown $wtd_user $wtd_home
mkdir -p $wtd_home/Son
mkdir -p $wtd_home/Dls
cp -r Son/* $wtd_home/Son
echo "done."
sleep 2

echo "Create Database and conf file"
sudo systemctl restart mariadb
CONFFILE=/etc/watchdogd.conf
if [ ! -f $CONFFILE ]
 then
    echo "Creating New watchdog database passwd"
    if [ "$database" = "oui" ]
    then 
      NEWPASSWORD=`openssl rand -base64 32`
      sed "/usr/local/etc/watchdogd.conf.sample" -e "s#dbpasstobechanged#$NEWPASSWORD#g" | \
    fi
    sed -e "s#usertobechanged#$wtd_user#g" | \
    sudo tee "$CONFFILE" > /dev/null
    if [ "$database" = "oui" ]
    then
     /usr/bin/mysqladmin -u root create WatchdogDB
     echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
     /usr/bin/mysql_secure_installation
    fi
fi
echo "done."
fi

echo "Starting Watchdog"
	sudo systemctl start Watchdogd.service
echo "done."

