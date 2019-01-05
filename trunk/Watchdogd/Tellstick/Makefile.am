#**********************************************************************************************************/
#* Watchdogd/Tellstick/Makefile.am          Profil de compilation de l'interface Tellstick pour Watchdog  */
#* Projet WatchDog version 3.0       Gestion d'habitat                           jeu 17 fév 2011 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-tellstick.la
libwatchdog_server_tellstick_la_SOURCES    = Tellstick.c Tellstick.h admin_tellstick.c

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

libwatchdog_server_tellstick_la_LDFLAGS = -ltelldus-core -rdynamic \
                                          `pkg-config --libs glib-2.0`
