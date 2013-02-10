#**********************************************************************************************************/
#* Watchdogd/Dls/Makefile.am          Profil de compilation de l'interface DLS Watchdog                   */
#* Projet WatchDog version 2.0       Gestion d'habitat                      ven 01 aoû 2003 11:37:48 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES      = libdls.la
libdls_la_SOURCES    = plugins.c heure.c Dls_db.c The_dls.c

libdls_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                    `mysql_config --libs` \
                    -rdynamic \
                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)


libdls_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                   `mysql_config --cflags`

INCLUDES=-I$(top_srcdir)/Include \
         -I../Include
