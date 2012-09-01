#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Serveur Dls TraductionDLS Rs485 Sms Archive Modbus Audio Admin \
          Utilisateur \
	  Synoptiques Onduleur Tellstick Lirc Asterisk Rfxcom Imsg

bin_PROGRAMS = Watchdogd
Watchdogd_SOURCES = Watchdogd.c process.c \
                    ecoute.c shm.c Config.c \
                    db.c ssl.c Mnemonique.c Message.c \
		    Compteur_horaire.c Compteur_impulsion.c \
		    Icones.c classe.c Scenario.c \
		    EntreeANA.c Histo.c Histo_hard.c Camera.c \
                    distrib_MSGxxx.c distrib_Ixxx.c

Watchdogd_LDADD   = Serveur/libserveur.la \
		    Dls/libdls.la \
		    TraductionDLS/libtradDLS.la \
		    Modbus/libmodbus.la \
		    Sms/libsms.la \
		    Archive/libarchive.la \
		    Modbus/libmodbus.la \
		    Audio/libaudio.la \
		    Admin/libadmin.la \
		    Utilisateur/libutil.la \
		    Synoptiques/libsyn.la \
		    Asterisk/libasterisk.la \
		    $(top_srcdir)/Commun/libwatchdog.la

Watchdogd_LDFLAGS = -lssl -lpopt `pkg-config --libs glib-2.0 gthread-2.0` `mysql_config --libs` $(GNOKII_LIBS) $(UPS_LIBS) $(CURL_LIBS)

sysconf_DATA = watchdogd.conf
pkgdata_DATA = init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc

INCLUDES=-I$(top_srcdir)/Include \
	 -IInclude/ \
	 `pkg-config --cflags glib-2.0 gthread-2.0` \
	 `mysql_config --cflags` \
	 $(CURL_CFLAGS) \
	 $(UPS_CFLAGS)

EXTRA_DIST = watchdogd.conf init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc
MAINTAINERCLEANFILES = Makefile.in
