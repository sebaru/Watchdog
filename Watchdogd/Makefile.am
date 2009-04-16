#**********************************************************************************************************/
#* Watchdogd/Makefile.am          Profil de compilation du serveur Watchdog                               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:24:24 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS = Include Config Db Serveur Dls TraductionDLS Rs485 Sms Archive Modbus Audio Admin

bin_PROGRAMS = Watchdogd
Watchdogd_SOURCES = Watchdogd.c process.c \
                    ecoute.c shm.c \
                    initdb.c ssl.c \
                    distrib_MSGxxx.c distrib_Ixxx.c

Watchdogd_LDADD   = Config/libconfig.la \
		    Db/libdb.la \
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
		    $(top_srcdir)/Commun/libwatchdog.la

Watchdogd_LDFLAGS = -lodbc -rdynamic -lssl -lpopt $(GLIB_LIBS) $(BONOBO_LIBS) $(GNOKII_LIBS) $(MYSQL_LIBS)

INCLUDES=-I$(top_srcdir)/Include \
	 -IInclude/ \
	 $(GLIB_CFLAGS) \
	 $(BONOBO_CFLAGS) \
	 $(MYSQL_CFLAGS)

MAINTAINERCLEANFILES = Makefile.in
