#!/bin/sh

usermod -a -G audio watchdog

echo "Creating systemd service"
ln -s /usr/local/etc/Watchdogd.service /etc/systemd/system/Watchdogd.service
systemctl daemon-reload
systemctl enable Watchdogd.service
echo "done."

echo "Creating etc conf file"
ln -s /usr/local/etc/watchdogd.conf.sample /etc/watchdogd.conf.sample
ln -s /usr/local/etc/watchdogd-httpd.conf.sample /etc/watchdogd-httpd.conf.sample
systemctl daemon-reload
echo "done."

sudo -u watchdog mkdir ~watchdog/Gif
sudo -u watchdog mkdir ~watchdog/Son
sudo -u mkdir ~watchdog/Dls

sudo -u watchdog cp -rv Gif/* ~watchdog/Gif
sudo -u watchdog cp -rv Son/* ~watchdog/Son

echo "Directory created and Files copied for SRV"

echo "Create Database"
/usr/bin/mysqladmin -u root -p create WatchdogDB
cat 'CREATE USER 'watchdog' IDENTIFIED BY 'watchdog'; GRANT ALL PRIVILEGES ON WatchdogDB.* TO watchdog; source /usr/local/share/Watchdog/init_db.sql;" | mysql -u root -p WatchdogDB
