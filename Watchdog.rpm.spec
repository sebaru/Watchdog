	
# This is a sample spec file for wget

%define _topdir	 	/mnt/Satis-storage/Partage/Distributions/Watchdog
%define name		Watchdog
%define release		1
%define version 	latest
Summary: 		Watchdog
License: 		GPL
Name: 			%{name}
Version: 		%{version}
Release: 		%{release}
Source: 		%{name}-%{version}.tar.gz
Prefix: 		/
Group: 			Development/Tools
Vendor:			A.B.L.S Corporation
Packager:		Sébastien LEFEVRE <lefevre.seb@gmail.com>

%description
Fichier de spécification pour la création des packages RPM server, client et partagé

%prep
%setup 

%build
./configure --prefix=%{buildroot} --exec-prefix=%{buildroot}/usr --includedir=%{buildroot}/usr/include --datarootdir=%{buildroot}/usr/share
make -j 4

%install
make -j 4 install

#---------------------------- Package Commun ---------------------------------
%package common
Summary: The Watchdogd common library
Group:                  Development/Tools
%description common
Common libraries for Client and Server !

%files common
%defattr(644,root,root)
/usr/lib/libwatchdog.*

#---------------------------- Package Client --------------------------------

%package client
Summary: The Watchdog client
Group:                  Development/Tools
requires: Watchdog-common
%description client
This is the client side of Watchdog


%files client
%defattr(755,root,root)
/usr/bin/Watchdog-client

#----------------------------- Package Server -------------------------------
%package server
Summary: The Watchdogd Server
Group:                  Development/Tools
requires: Watchdog-common gcc
%description server
This is the server side of Watchdog

%files server
%defattr(644,root,root)
/usr/include/*
/usr/lib/libdls*
/usr/share/Watchdog/init_db.sql
/usr/share/Watchdog/openssl.cnf
%attr(755,root,root) /usr/bin/Watchdogd
%attr(755,root,root) /usr/bin/WatchdogdAdmin
%attr(640,root,watchdog) %config(noreplace) /etc/watchdogd.conf
#%config /etc/watchdogd.conf
%doc /usr/share/Watchdog/Watchdog*pdf
%doc /usr/share/Watchdog/Delete_old_avi.sh
%doc /usr/share/Watchdog/Programmateur_mysql.sql
%doc /usr/share/Watchdog/motion.conf
%doc /usr/share/Watchdog/cam0001.conf

#----------------------------- Package Server-rfxcom -------------------------------
%package server-rfxcom
Summary: The Watchdogd Server - Module RFXCom
Group:                  Development/Tools
requires: Watchdog-server
%description server-rfxcom
This is the server side of Watchdog - RFXCOM Module

%files server-rfxcom
%defattr(644,root,root)
/usr/lib/libwatchdog-rfxcom*

#----------------------------- Package Server-rs485 -------------------------------
%package server-rs485
Summary: The Watchdogd Server - Module RS485
Group:                  Development/Tools
requires: Watchdog-server
%description server-rs485
This is the server side of Watchdog - RS485 Module

%files server-rs485
%defattr(644,root,root)
/usr/lib/libwatchdog-rs485*

#----------------------------- Package Server-tellstick -------------------------------
%package server-tellstick
Summary: The Watchdogd Server - Module tellstick
Group:                  Development/Tools
requires: Watchdog-server
%description server-tellstick
This is the server side of Watchdog - Telldus Module

%files server-tellstick
%defattr(644,root,root)
/usr/lib/libwatchdog-tellstick*

#----------------------------- Package Server-lirc -----------------------------------
%package server-lirc
Summary: The Watchdogd Server - Module lirc
Group:                  Development/Tools
requires: Watchdog-server
%description server-lirc
This is the server side of Watchdog - Lirc Module

%files server-lirc
%defattr(644,root,root)
/usr/share/Watchdog/lircrc
/usr/lib/libwatchdog-lirc*

#----------------------------- Package Server-rfxcom ----------------------------------
%package server-rfxcom
Summary: The Watchdogd Server - Module RFXCOM
Group:                  Development/Tools
requires: Watchdog-server
%description server-rfxcom
This is the server side of Watchdog - RFXCOM Module

%files server-rfxcom
%defattr(644,root,root)
/usr/lib/libwatchdog-server-rfxcom*

