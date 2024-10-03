#**********************************************************************************************************/
#* Watchdogd/Dls/Makefile.am          Profil de compilation de l'interface DLS Watchdog                   */
#* Projet Abls-Habitat version 4.0       Gestion d'habitat                 ven 01 aout 2003 11:37:48 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES            = libwatchdog-dls.la
libwatchdog_dls_la_SOURCES = plugins.c heure.c The_dls.c The_dls_archive.c The_dls_data.c \
                             The_dls_AI.c The_dls_AO.c The_dls_DI.c The_dls_DO.c The_dls_CH.c The_dls_CI.c The_dls_MESSAGE.c \
                             The_dls_REGISTRE.c The_dls_WATCHDOG.c The_dls_HORLOGE.c The_dls_TEMPO.c The_dls_BI.c The_dls_MONO.c \
                             The_dls_VISUEL.c

libwatchdog_dls_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0` \
                    -rdynamic \
                    -version-info $(LIBDLS_MAJOR_VERSION):$(LIBDLS_MINOR_VERSION):$(LIBDLS_MICRO_VERSION)

libwatchdog_dls_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0 json-glib-1.0 libsoup-3.0`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include
