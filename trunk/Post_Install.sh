#!/bin/sh

echo "Creating systemd service"
if [ "$1" = "server" ]
then
        wtd_home=/home/watchdog
        wtd_user=watchdog
	echo "Installation in standalone mode in $wtd_home for $wtd_user"
        sudo cp /usr/local/etc/Watchdogd.service.system /etc/systemd/system/Watchdogd.service
        sudo systemctl daemon-reload
        sudo systemctl enable Watchdogd.service
        sudo usermod -a -G audio,dialout,wheels $wtd_user
else
	wtd_home=~/.watchdog
        wtd_user=`whoami`
	echo "Installation in user mode in $wtd_home for $wtd_user"
        sudo cp /usr/local/etc/Watchdogd.service.user /etc/systemd/user/Watchdogd.service
        sudo systemctl daemon-reload
        systemctl --user enable Watchdogd.service
        sudo usermod -a -G audio,dialout $wtd_user
fi
sudo systemctl daemon-reload
echo "done."
sleep 2

echo "Enabling pulseaudio systemd service"
if [ "$1" = "server" ]
then
	loginctl enable-linger $wtd_user
fi
systemctl --user enable pulseaudio
systemctl --user start pulseaudio
echo "done."
sleep 2

echo "Copying data files"
mkdir -p $wtd_home
mkdir -p $wtd_home/Son
mkdir -p $wtd_home/Dls
cp Watchdogd/Voice/fr.dict $wtd_home/
cp Watchdogd/Voice/wtd.gram $wtd_home/
cp -r Watchdogd/Voice/cmusphinx-fr-5.2 $wtd_home/
cp -r Son/* $wtd_home/Son
mkdir -p $wtd_home/.pulse/
echo "done."
sleep 2

if [ "$1" = "server" ]
then
echo "Create Database and conf file"
sudo systemctl restart mariadb
CONFFILE=/etc/watchdogd.conf
if [ ! -f $CONFFILE ]
 then
    echo "Creating New watchdog database passwd"
    NEWPASSWORD=`openssl rand -base64 32`
    sed "/usr/local/etc/watchdogd.conf.sample" -e "s#dbpasstobechanged#$NEWPASSWORD#g" | \
    sed -e "s#hometobechanged#$wtd_home#g" | \
    sed -e "s#usertobechanged#$wtd_user#g" | \
    sudo tee "$CONFFILE" > /dev/null
   /usr/bin/mysqladmin -u root create WatchdogDB
   echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
   /usr/bin/mysql_secure_installation
fi
echo "done."
fi

echo "Starting Watchdog"
if [ "$1" = "server" ]
then
	sudo systemctl start Watchdogd.service
else
	systemctl --user start Watchdogd.service
fi
echo "done."

