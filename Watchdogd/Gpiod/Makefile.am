#**********************************************************************************************************/
#* Watchdogd/Gpiod/Makefile.am     Profil de compilation de l'interface RASPBERRYPI GPIOD pour Watchdog   */
#* Projet Abls-Habitat version 4.5       Gestion d'habitat                          ven 03 septembre 2021 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS=

lib_LTLIBRARIES                         = libwatchdog-server-gpiod.la
libwatchdog_server_gpiod_la_SOURCES    = Gpiod.c Gpiod.h

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_gpiod_la_LDFLAGS = -rdynamic \
                                      `pkg-config --libs glib-2.0 json-glib-1.0 libgpiod`

libwatchdog_server_gpiod_la_CFLAGS = `pkg-config --cflags glib-2.0 json-glib-1.0 libgpiod`


