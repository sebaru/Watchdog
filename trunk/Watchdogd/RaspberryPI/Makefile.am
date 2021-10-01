#**********************************************************************************************************/
#* Watchdogd/RaspberryPI/Makefile.am     Profil de compilation de l'interface RASPBERRYPI pour Watchdog   */
#* Projet WatchDog version 3.0       Gestion d'habitat                              ven 03 septembre 2021 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES                         = libwatchdog-server-raspberrypi.la
libwatchdog_server_raspberrypi_la_SOURCES    = RaspberryPI.c RaspberryPI.h admin_raspberryPI.c

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_raspberrypi_la_LDFLAGS = -rdynamic \
                                            `pkg-config --libs glib-2.0 json-glib-1.0 libsoup-2.4 libgpiod` \
                                            `mysql_config --libs`

libwatchdog_server_raspberrypi_la_CFLAGS = `pkg-config --cflags glib-2.0 json-glib-1.0 libsoup-2.4 libgpiod` \
                                           `mysql_config --cflags`

