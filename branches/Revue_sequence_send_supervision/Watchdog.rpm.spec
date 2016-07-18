	
# This is a sample spec file for wget

%define _topdir	 	/home/sebastien/Essai/Watchdog
%define name		Watchdog
%define version 	2.8.latest
%define release         1
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
make -j 1

%install
make -j 1 install

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
/usr/lib/libwatchdog-dls*
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

#----------------------------- Package Server-ssrv ----------------------------------
%package server-ssrv
Summary: The Watchdogd Server - Module SSRV
Group:                  Development/Tools
requires: Watchdog-server
%description server-ssrv
This is the server side of Watchdog - SSRV Module

%files server-ssrv
%defattr(644,root,root)
/usr/lib/libwatchdog-server-ssrv*

#----------------------------- Package Server-sms ----------------------------------
%package server-sms
Summary: The Watchdogd Server - Module SMS
Group:                  Development/Tools
requires: Watchdog-server
%description server-sms
This is the server side of Watchdog - SMS Module

%files server-sms
%defattr(644,root,root)
/usr/lib/libwatchdog-server-sms*

#----------------------------- Package Server-modbus ------------------------------
%package server-modbus
Summary: The Watchdogd Server - Module MODBUS
Group:                  Development/Tools
requires: Watchdog-server
%description server-modbus
This is the server side of Watchdog - MODBUS (Wago) Module

%files server-modbus
%defattr(644,root,root)
/usr/lib/libwatchdog-server-modbus*

#----------------------------- Package Server-rs485 -------------------------------
%package server-rs485
Summary: The Watchdogd Server - Module RS485
Group:                  Development/Tools
requires: Watchdog-server
%description server-rs485
This is the server side of Watchdog - RS485 Module

%files server-rs485
%defattr(644,root,root)
/usr/lib/libwatchdog-server-rs485*

#----------------------------- Package Server-ups -------------------------------
%package server-ups
Summary: The Watchdogd Server - Module UPS
Group:                  Development/Tools
requires: Watchdog-server
%description server-ups
This is the server side of Watchdog - UPS Module

%files server-ups
%defattr(644,root,root)
/usr/lib/libwatchdog-server-ups*

#----------------------------- Package Server-tellstick -------------------------------
#%package server-tellstick
#Summary: The Watchdogd Server - Module tellstick
#Group:                  Development/Tools
#requires: Watchdog-server
#%description server-tellstick
#This is the server side of Watchdog - Telldus Module
#
#%files server-tellstick
#%defattr(644,root,root)
#/usr/lib/libwatchdog-server-tellstick*

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
/usr/lib/libwatchdog-server-lirc*

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

#----------------------------- Package Server-audio ----------------------------------
%package server-audio
Summary: The Watchdogd Server - Module AUDIO
Group:                  Development/Tools
requires: Watchdog-server
%description server-audio
This is the server side of Watchdog - AUDIO Module

%files server-audio
%defattr(644,root,root)
/usr/lib/libwatchdog-server-audio*

#----------------------------- Package Server-imsg ----------------------------------
%package server-imsg
Summary: The Watchdogd Server - Instant Messaging Thread
Group:                  Development/Tools
requires: Watchdog-server
%description server-imsg
This is the server side of Watchdog - Instant Messaging Module

%files server-imsg
%defattr(644,root,root)
/usr/lib/libwatchdog-server-imsg*

#----------------------------- Package Server-teleinfoedfusb ----------------------------------
%package server-teleinfoedf
Summary: The Watchdogd Server - Teleinfo EDF Thread
Group:                  Development/Tools
requires: Watchdog-server
%description server-teleinfoedf
This is the server side of Watchdog - Teleinfo EDF Module

%files server-teleinfoedf
%defattr(644,root,root)
/usr/lib/libwatchdog-server-teleinfoedf*

#----------------------------- Package Server-Satellite ----------------------------------
%package server-satellite
Summary: The Watchdogd Server - Satellite Thread for sending Internal Data
Group:                  Development/Tools
requires: Watchdog-server
%description server-satellite
This is the server side of Watchdog - Satellite Thread for Sending Internal Data

%files server-satellite
%defattr(644,root,root)
/usr/lib/libwatchdog-server-satellite*

#----------------------------- Package Server-HTTP ----------------------------------
%package server-http
Summary: The Watchdogd Server - HTTP Thread for requesting and setting Internal Data
Group:                  Development/Tools
requires: Watchdog-http
%description server-http
This is the server side of Watchdog - HTTP Thread for Requesting/Setting Internal Data

%files server-http
%defattr(644,root,root)
/usr/lib/libwatchdog-server-http*

