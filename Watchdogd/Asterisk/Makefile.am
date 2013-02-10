#**********************************************************************************************************/
#* Watchdogd/Asterisk/Makefile.am          Profil de compilation de la librairies de gestion Asterisk     */
#* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 28 mai. 2011 18:53:00 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libasterisk.la
libasterisk_la_SOURCES = Asterisk.c


libasterisk_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                         `mysql_config --libs`

libasterisk_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                        `mysql_config --cflags`

INCLUDES=-I$(top_srcdir)/Include \
         -I../Include
