#**********************************************************************************************************/
#* Watchdogd/Db/Dls/Makefile.am    Profil de compilation de la librairies de gestion des prgs DLS         */
#* Projet WatchDog version 2.0       Gestion d'habitat                      lun 07 avr 2003 22:43:42 CEST */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

noinst_LTLIBRARIES = libdls.la
libdls_la_SOURCES = Dls_db.c

INCLUDES= -I$(top_srcdir) -I$(includedir) \
          $(GLIB_CFLAGS) \
          $(BONOBO_CFLAGS) \
          -I$(top_srcdir)/Include \
          -I$(top_srcdir)/intl \
	  -I../../Include
