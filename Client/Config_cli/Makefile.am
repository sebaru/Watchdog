#**********************************************************************************************************/
#* Client/Config_cli/Makefile.am          Profil de compilation du l'outil de configuration du client      */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:41:53 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libconfig_cli.la
libconfig_cli_la_SOURCES = Config_cli_lignes.y Config_cli_mots.l Config_cli.c

#Pour LEX et YACC
libconfig_cli_la_LIBADD  = @LEXLIB@


INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
          -I../Include \
          $(GLIB_CFLAGS)

CLEANFILES = Config_cli_lignes.h Config_cli_lignes.c Config_cli_mots.c
