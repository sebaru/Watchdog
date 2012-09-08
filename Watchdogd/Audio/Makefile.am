#**********************************************************************************************************/
#* Watchdogd/Audio/Makefile.am          Profil de compilation de l'interface Audio Watchdog               */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mar 11 avr 2009 15:48:08 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

lib_LTLIBRARIES     = libwatchdog-server-audio.la
libwatchdog_server_audio_la_SOURCES    = Audio.c Audio.h admin_audio.c

INCLUDES= `pkg-config --cflags glib-2.0` \
	  `mysql_config --cflags` \
          -I$(top_srcdir)/Include \
 	  -I../Include
