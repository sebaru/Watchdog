/**********************************************************************************************************/
/* Watchdogd/Include/Cpth_DB.h        Déclaration structure internes des compteurs horaire                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 14 fév 2006 15:18:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CPTH_H_
 #define _CPTH_H_

 #include "Db.h"

 #define NOM_TABLE_CPTH    "dls_cpth"

 struct CPTH_DB
  { guint  id;                                                                      /* Numero du compteur */
    guint  valeur;                                                                  /* Valeur du compteur */
  };

 struct CPT_HORAIRE
  { struct CPTH_DB cpthdb;
    guint old_top;
    gboolean actif;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Updater_cpthDB ( struct LOG *log, struct DB *db, struct CPTH_DB *val );
 extern struct CPTH_DB *Rechercher_cpthDB( struct LOG *log, struct DB *db, guint id );

 extern gboolean Creer_db_cpth ( struct LOG *log, struct DB *db );
#endif
/*--------------------------------------------------------------------------------------------------------*/
