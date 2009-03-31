#**********************************************************************************************************/
#* Watchdogd/TraductionDLS/Makefile.am          Profil de compilation de l'outil de traduction DLS        */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mer 06 aoû 2003 15:33:14 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libtradDLS.la
libtradDLS_la_SOURCES = lignes.y mots.l Interp.c
AM_LFLAGS = -PDls_ -olex.yy.c
AM_YFLAGS = -p Dls_

#Pour LEX et YACC
libtradDLS_la_LIBADD  = @LEXLIB@


INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(GLIB_CFLAGS)

CLEANFILES = lignes.h lignes.c mots.c
