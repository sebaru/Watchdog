#**********************************************************************************************************/
#* Watchdogd/Config/Makefile.am          Profil de compilation de l'outil de configuration                */
#* Projet WatchDog version 2.6       Gestion d'habitat                      ven 10 dec 2010 16:57:23 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libconfig.la
libconfig_la_SOURCES = Config.c

#Pour LEX et YACC
#libconfig_la_LIBADD  = @LEXLIB@


INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
	  $(MYSQL_CFLAGS) \
          $(GLIB_CFLAGS)

CLEANFILES = 
