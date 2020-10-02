#!/bin/sh

SOCLE=`grep "^ID" /etc/os-release | cut -f 2 -d '='`
USER=`whoami`

sh INSTALL_without_SGBD.sh

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

