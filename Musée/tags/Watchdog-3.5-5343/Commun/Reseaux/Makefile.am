#**********************************************************************************************************/
#* Commun/Reseaux/Makefile.am          Profil de compilation de librairies REZO                           */
#* Projet WatchDog version 3.0       Gestion d'habitat                      mer 04 jun 2003 13:54:13 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = librezo.la
librezo_la_SOURCES = Reseaux.c Ssl.c
#libutil_la_LIBADD  = $(GNOMEUI_LIBS) -lmcrypt

AM_CPPFLAGS= -I$(top_srcdir) -I$(includedir) \
          $(GLIB_CFLAGS) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl
