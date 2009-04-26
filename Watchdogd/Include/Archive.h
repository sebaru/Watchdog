/**********************************************************************************************************/
/* Watchdogd/Include/Archivage_DB.h        Déclaration structure internes des archivages                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 08 jui 2006 12:02:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ARCHIVAGE_DB_H_
 #define _ARCHIVAGE_DB_H_

 #include "Db.h"

 #define NOM_TABLE_ARCH    "histo_bit"

 struct ARCHDB
  { guint  date_sec;                                                                  /* Date de la photo */
    guint  date_usec;                                                                 /* Date de la photo */
    guint  type;                                                             /* Type de bit: E ? B ? EA ? */
    guint  num;                                            /* Numero de l'entrée analogique photographiée */
    guint  valeur;                                                       /* Valeur de l'entrée analogique */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_arch ( void );                                                         /* Dans Archive.c */
 extern void Ajouter_arch( gint type, gint num, gint valeur );
 extern gboolean Recuperer_archDB ( struct LOG *log, struct DB *db, guint type, guint num,
                                    time_t date_deb, time_t date_fin );
 extern struct ARCHDB *Recuperer_archDB_suite( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
