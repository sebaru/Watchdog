#**********************************************************************************************************/
#* Watchdogd/TraductionDLS/Makefile.am          Profil de compilation de l'outil de traduction DLS        */
#* Projet WatchDog version 2.0       Gestion d'habitat                      mer 06 ao� 2003 15:33:14 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES   = libtradDLS.la
libtradDLS_la_SOURCES = lignes.y mots.l Interp.c
AM_LFLAGS = -PDls_ -olex.yy.c
AM_YFLAGS = -p Dls_

#Pour LEX et YACC
libtradDLS_la_LIBADD  = @LEXLIB@

libtradDLS_la_LDFLAGS = `pkg-config --libs glib-2.0 gthread-2.0` \
                        `mysql_config --libs`

libtradDLS_la_CFLAGS = `pkg-config --cflags glib-2.0 gthread-2.0` \
                       `mysql_config --cflags`

AM_CPPFLAGS=-I$(top_srcdir)/Include \
            -I$(top_srcdir)/Watchdogd/Include

EXTRA_DIST = lignes.h
CLEANFILES = lignes.h lignes.c mots.c
