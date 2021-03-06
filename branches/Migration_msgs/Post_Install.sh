#!/bin/sh

echo "Creating watchdog user"
useradd watchdog
usermod -a -G audio watchdog
echo "done."
sleep 2

echo "Creating systemd service"
ln -s /usr/local/etc/Watchdogd.service /etc/systemd/system/Watchdogd.service
systemctl enable Watchdogd.service
systemctl daemon-reload
echo "done."
sleep 2

echo "Copying data files"
sudo -u watchdog mkdir ~watchdog/Gif
sudo -u watchdog mkdir ~watchdog/Son
sudo -u watchdog mkdir ~watchdog/Dls

sudo -u watchdog cp -r Gif/* ~watchdog/Gif
sudo -u watchdog cp -r Son/* ~watchdog/Son

echo "done."
sleep 2

echo "Create Database and conf file"
systemctl restart mariadb
CONFFILE=/etc/watchdogd.conf
if [ ! -f $CONFFILE ];
 then
    echo "Creating New watchdog database passwd"
    NEWPASSWORD=`openssl rand -base64 32`
    sed "/usr/local/etc/watchdogd.conf.sample" -e "s#tobechanged#$NEWPASSWORD#g" > "$CONFFILE"
   /usr/bin/mysqladmin -u root create WatchdogDB
   echo "CREATE USER 'watchdog' IDENTIFIED BY '$NEWPASSWORD'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; FLUSH PRIVILEGES; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root WatchdogDB
   /usr/bin/mysql_secure_installation
fi
echo "done."

echo "Starting Watchdog"
systemctl start Watchdogd.service
echo "done."

