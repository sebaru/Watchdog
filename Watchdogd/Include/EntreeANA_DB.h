/**********************************************************************************************************/
/* Watchdogd/Include/EntreeANA.h        Déclaration structure internes des entree analogiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 11:17:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ENTREEANA_DB_H_
 #define _ENTREEANA_DB_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_entreeana.h"

 #define NOM_TABLE_ENTREEANA    "eana"

 struct ENTREEANA_DB
  { guint  num;                                                                         /* Numero de l'EA */
    guint  unite;                                                                           /* °C, °K, Km */
    gfloat min;                                                       /* Etat, prealarme, defaut, alarme */
    gfloat max;
    guchar libelle[ NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1 ];
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_eana ( void );
 extern struct ENTREEANA_DB *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint num );
 extern gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db );
 extern struct ENTREEANA_DB *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_ENTREEANA *entreeana );
#endif
/*--------------------------------------------------------------------------------------------------------*/
