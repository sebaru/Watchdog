#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Config Serveur Dls TraductionDLS Rs485 Sms Archive Modbus Audio Admin \
          CptHoraire EntreeANA Message Mnemonique Scenario Icones Histo Utilisateur \
	  Synoptiques Camera Onduleur

bin_PROGRAMS = Watchdogd
Watchdogd_SOURCES = Watchdogd.c process.c \
                    ecoute.c shm.c \
                    db.c ssl.c lirc.c \
                    distrib_MSGxxx.c distrib_Ixxx.c

Watchdogd_LDADD   = Config/libconfig.la \
		    Serveur/libserveur.la \
		    Dls/libdls.la \
		    TraductionDLS/libtradDLS.la \
		    Rs485/librs485.la \
		    Modbus/libmodbus.la \
		    Sms/libsms.la \
		    Archive/libarchive.la \
		    Modbus/libmodbus.la \
		    Audio/libaudio.la \
		    Admin/libadmin.la \
		    CptHoraire/libcpth.la \
		    EntreeANA/libeana.la \
		    Message/libmsg.la \
		    Mnemonique/libmnemo.la \
		    Scenario/libscenario.la \
		    Icones/libico.la \
		    Histo/libhisto.la \
		    Utilisateur/libutil.la \
		    Synoptiques/libsyn.la \
		    Camera/libcamera.la \
		    Onduleur/libonduleur.la \
		    $(top_srcdir)/Commun/libwatchdog.la

Watchdogd_LDFLAGS = -rdynamic -lssl -lpopt $(GLIB_LIBS) $(BONOBO_LIBS) $(GNOKII_LIBS) $(MYSQL_LIBS) $(UPS_LIBS) $(CURL_LIBS) $(LIRC_LIBS)

INCLUDES=-I$(top_srcdir)/Include \
	 -IInclude/ \
	 $(GLIB_CFLAGS) \
	 $(BONOBO_CFLAGS) \
	 $(MYSQL_CFLAGS) \
	 $(LIRC_CFLAGS) \
	 $(CURL_CFLAGS) \
	 $(UPS_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
