#!/bin/sh

echo "Creating systemd service"

echo "Quel user fera tourner Watchdog ?"
read -p "User: " wtd_user

echo "L'instance est-elle master ? (oui/non)"
read -p "instance_is_master: " master

if [ "$wtd_user" = "watchdog" ]
	then
		wtd_home=~$wtd_user
	else
		wtd_home=~$wtd_user/.watchdog
fi

echo "Installation in standalone mode in $wtd_home for $wtd_user"
sleep 5
if [ "$master" = "oui" ]
	then
		sudo cp /usr/local/etc/Watchdogd.service.master /etc/systemd/system/Watchdogd.service
	else
                sudo cp /usr/local/etc/Watchdogd.service.slave /etc/systemd/system/Watchdogd.service
fi
sudo systemctl daemon-reload
sudo systemctl enable Watchdogd.service
sudo usermod -a -G audio,dialout $wtd_user
sudo loginctl enable-linger $wtd_user
sudo systemctl daemon-reload
echo "done."
sleep 2

echo "Copying data files"
sudo mkdir -p $wtd_home
sudo mkdir -p $wtd_home/Son
sudo mkdir -p $wtd_home/Dls
sudo cp -r Son/* $wtd_home/Son
sudo chown -R $wtd_user:users $wtd_home
echo "done."
sleep 2

echo "Create Database and conf file"
CONFFILE=/etc/watchdogd.conf
if [ ! -f $CONFFILE ]
 then
    echo "Creating New watchdog database passwd"
    NEWPASSWORD=`openssl rand -base64 32`
    sed "/usr/local/etc/watchdogd.conf.sample" -e "s#dbpasstobechanged#$NEWPASSWORD#g" | \
    sed -e "s#usertobechanged#$wtd_user#g" | \
    sudo tee "$CONFFILE" > /dev/null
    if [ "$master" = "oui" ]
    then
    sudo systemctl restart mariadb
    /usr/bin/mysqladmin -u root create WatchdogDB
     echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
     /usr/bin/mysql_secure_installation
    fi
fi
echo "done."

echo "Starting Watchdog"
	sudo ldconfig	
	sudo systemctl start Watchdogd.service
echo "done."

