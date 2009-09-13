#**********************************************************************************************************/
#* Watchdogd/Camera/Makefile.am          Profil de compilation de la librairies de gestion camera         */
#* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:57:22 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libcamera.la
libcamera_la_SOURCES = Camera.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
	  $(MYSQL_CFLAGS) \
          $(GLIB_CFLAGS)

