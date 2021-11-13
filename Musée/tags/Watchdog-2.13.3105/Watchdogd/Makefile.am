#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Serveur Dls TraductionDLS Archive Enocean \
	  Rs485 Sms Modbus Audio Onduleur Lirc Rfxcom Imsg Satellite TeleinfoEDF Http

bin_PROGRAMS = Watchdogd
Watchdogd_SOURCES = Watchdogd.c process.c \
                    shm.c Config.c Mnemo_Registre.c \
                    db.c Mnemonique.c Mnemo_AI.c Mnemo_CPT_IMP.c Mnemo_CPT_H.c Message.c \
		    Icones.c classe.c \
		    Mnemo_Tempo.c Histo_msgs.c Camera.c distrib_Events.c \
                    distrib_MSGxxx.c distrib_Ixxx.c \
                    Admin.c Admin_running.c Admin_process.c Admin_dls.c Admin_set.c Admin_get.c Admin_dbcfg.c Admin_user.c \
                    Utilisateur.c Util_groupe.c Util_login_failed.c Util_actif.c \
                    Synoptiques.c Syn_motifs.c Syn_comment.c Syn_passerelle.c Syn_palette.c Syn_cadran.c Syn_camera_sup.c

Watchdogd_LDADD   = Dls/libwatchdog-dls.la \
		    TraductionDLS/libtradDLS.la \
		    Archive/libarchive.la \
		    $(top_builddir)/Commun/libwatchdog.la

# rdynamic pour autoriser l'utilsation des fonctions du thread principal par les librairies dlopened
Watchdogd_LDFLAGS = -rdynamic -lpopt \
                   `pkg-config --libs glib-2.0 gthread-2.0 libcurl openssl libxml-2.0` \
                   `libgcrypt-config --libs` \
                   `mysql_config --libs`

Watchdogd_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 libcurl openssl libxml-2.0` \
                   `libgcrypt-config --cflags` \
                   `mysql_config --cflags`

sysconf_DATA = watchdogd.conf.sample Watchdogd.service watchdogd-httpd.conf.sample
pkgdata_DATA = init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

EXTRA_DIST = watchdogd-http.conf.sample Watchdogd.service watchdogd.conf init_db.sql openssl.cnf Programmateur_mysql.sql Delete_old_avi.sh lircrc
MAINTAINERCLEANFILES = Makefile.in
