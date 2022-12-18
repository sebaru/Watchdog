#**********************************************************************************************************/
#* Watchdogd/Http/Makefile.am          Profil de compilation de l'interface HTTP Watchdog                 */
#* Projet WatchDog version 3.0       Gestion d'habitat                      ven 01 aoû 2003 11:37:48 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

noinst_LTLIBRARIES           = libwatchdog-http.la
libwatchdog_http_la_SOURCES  = Http.c getstatus.c bus_master.c bus_slave.c \
                               gethisto_msgs.c getdls.c getsyn.c \
                               getusers.c getfile.c


libwatchdog_http_la_LDFLAGS = `pkg-config --libs glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-2.4` \
                              `mariadb_config --libs` \
                              -rdynamic


libwatchdog_http_la_CFLAGS = `pkg-config --cflags glib-2.0 openssl gthread-2.0 json-glib-1.0 libsoup-2.4` \
                               `mariadb_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include \
            -DWTD_PKGDATADIR=\"$(pkgdatadir)\"
