#**********************************************************************************************************/
#* Watchdogd/Db/Makefile.am          Profil de compilation de l'outil de gestion SQL                      */
#* Projet WatchDog version 2.0       Gestion d'habitat                      dim 29 aoû 2004 20:41:47 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/
SUBDIRS= Message Utilisateur Dls Histo Icones Synoptiques Mnemonique EntreeANA ValANA CptHoraire Archive Scenario

noinst_LTLIBRARIES   = libdb.la
libdb_la_SOURCES     = Db.c
libdb_la_LIBADD      = Message/libmsg.la Utilisateur/libutil.la Dls/libdls.la \
                       Histo/libhisto.la Icones/libico.la Synoptiques/libsyn.la \
		       Mnemonique/libmnemo.la EntreeANA/libeana.la \
		       ValANA/libvalana.la \
		       Archive/libarchive.la \
                       CptHoraire/libcpth.la \
		       Scenario/libscenario.la

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../Include \
          $(GLIB_CFLAGS)

