#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Serveur Dls TraductionDLS Utilisateur Synoptiques Archive \
	  Rs485 Sms Modbus Audio Admin Onduleur Lirc Asterisk Rfxcom Imsg Master TeleinfoEDF

bin_PROGRAMS = Watchdogd
Watchdogd_SOURCES = Watchdogd.c process.c \
                    ecoute.c shm.c Config.c \
                    db.c ssl.c Mnemonique.c Message.c \
		    Compteur_horaire.c Compteur_impulsion.c \
		    Icones.c classe.c Scenario.c \
		    EntreeANA.c Histo.c Histo_hard.c Camera.c \
                    distrib_MSGxxx.c distrib_Ixxx.c distrib_Axxx.c 

Watchdogd_LDADD   = Serveur/libserveur.la \
		    Dls/libdls.la \
		    TraductionDLS/libtradDLS.la \
		    Archive/libarchive.la \
		    Admin/libadmin.la \
		    Utilisateur/libutil.la \
		    Synoptiques/libsyn.la \
		    Asterisk/libasterisk.la \
		    $(top_srcdir)/Commun/libwatchdog.la

# rdynamic pour autoriser l'utilsation des fonctions du thread principal par les librairies dlopened
Watchdogd_LDFLAGS = -rdynamic -lpopt \
                   `pkg-config --libs glib-2.0 gthread-2.0` \
                   `mysql_config --libs`

Watchdogd_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                   `mysql_config --cflags`

sysconf_DATA = watchdogd.conf
pkgdata_DATA = init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc motion.conf cam0001.conf

INCLUDES=-I$(top_srcdir)/Include \
	 -IInclude

EXTRA_DIST = watchdogd.conf init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc motion.conf cam0001.conf
MAINTAINERCLEANFILES = Makefile.in
