#**********************************************************************************************************/
#* Watchdogd/Config/Makefile.am          Profil de compilation de l'outil de configuration                */
#* Projet WatchDog version 1.7       Gestion d'habitat                      lun 02 jun 2003 14:41:53 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libconfig.la
libconfig_la_SOURCES = Config_lignes.y Config_mots.l Config.c
AM_LFLAGS = -PConfig_ -olex.yy.c
AM_YFLAGS = -p Config_

#Pour LEX et YACC
libconfig_la_LIBADD  = @LEXLIB@


INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(GLIB_CFLAGS)

CLEANFILES = Config_lignes.h Config_lignes.c Config_mots.c
