#**********************************************************************************************************/
#* CommunClient/Makefile.am          Profil de compilation des librairies communes Client Watchdog2.0     */
#* Projet WatchDog version 2.0       Gestion d'habitat                     lun. 09 nov. 2009 12:09:30 CET */
#* Auteur: LEFEVRE Sebastien                                                                              */
#**********************************************************************************************************/

lib_LTLIBRARIES = libwatchdogcli.la
libwatchdogcli_la_SOURCES = trame.c

libwatchdogcli_la_LDFLAGS = -version-info $(LIBW_MAJOR_VERSION):$(LIBW_MINOR_VERSION):$(LIBW_MICRO_VERSION)

libwatchdogcli_la_LIBADD  = $(GNOME2_LIBS)

INCLUDES=$(GNOME_INCLUDEDIR) \
         -I$(top_srcdir)/Include \
         -I$(top_srcdir)/IncludeClient \
         $(GNOME2_CFLAGS) 

MAINTAINERCLEANFILES = Makefile.in
