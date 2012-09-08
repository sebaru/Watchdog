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

libwatchdog_server_audio_la_LDFLAGS = -rdynamic -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION) \
	                              `pkg-config --libs glib-2.0`
