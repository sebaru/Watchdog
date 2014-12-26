#**********************************************************************************************************/
#* Watchdogd/Dls/Makefile.am          Profil de compilation de l'interface DLS Watchdog                   */
#* Projet WatchDog version 2.0       Gestion d'habitat                      ven 01 aoû 2003 11:37:48 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES      = libwatchdog-dls.la
libwatchdog_dls_la_SOURCES    = plugins.c heure.c Dls_db.c The_dls.c

libwatchdog_dls_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                    `mysql_config --libs` \
                    -rdynamic \
                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)


libwatchdog_dls_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                            `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
