#**********************************************************************************************************/
#* Watchdogd/Db/Utilisateur/Makefile.am    Profil de compilation de la librairies de gestion utilisateurs */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 07 avr 2003 22:43:42 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libutil.la
libutil_la_SOURCES = Utilisateur.c groupe.c login_failed.c \
                     clef.c actif.c proto_util.h

INCLUDES= `pkg-config --cflags glib-2.0` \
	  `mysql_config --cflags` \
          -I$(top_srcdir)/Include \
 	  -I../Include
