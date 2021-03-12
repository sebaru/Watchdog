#**********************************************************************************************************/
#* Client/Config_cli/Makefile.am          Profil de compilation du l'outil de configuration du client      */
#* Projet WatchDog version 3.0       Gestion d'habitat                      lun 02 jun 2003 14:41:53 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libconfig_cli.la
libconfig_cli_la_SOURCES = Config_cli.c

AM_CPPFLAGS= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
          -I$(top_srcdir)/ClientGTK2/Include \
          $(GLIB_CFLAGS)

CLEANFILES = 
