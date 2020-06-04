sudo apt -y install php php7.3-mysql
sudo a2enmod headers
sudo a2enmod rewrite
sudo a2enmod proxy
sudo a2enmod proxy_wstunnel
sudo cp watchdogd-httpd.conf /etc/apache2/sites-available/
