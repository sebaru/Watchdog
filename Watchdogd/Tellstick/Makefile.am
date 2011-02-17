#**********************************************************************************************************/
#* Watchdogd/Tellstick/Makefile.am          Profil de compilation de l'interface Tellstick pour Watchdog  */
#* Projet WatchDog version 2.0       Gestion d'habitat                           jeu 17 fév 2011 18:13:27 */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= 

noinst_LTLIBRARIES     = libtellstick.la
libtellstick_la_SOURCES    = Tellstick.c

INCLUDES= $(GNOME2_CFLAGS) \
	  $(MYSQL_CFLAGS) \
	  -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include

libtellstick_la_LDFLAGS = -ltelldus-core
