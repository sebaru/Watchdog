/**********************************************************************************************************/
/* Watchdogd/Include/EntreeANA.h        Déclaration structure internes des entree analogiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 11:17:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CMD_TYPE_ENTREEANA_H_
 #define _CMD_TYPE_ENTREEANA_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_entreeana.h"

 #define NOM_TABLE_ENTREEANA    "eana"

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_eana ( void );
 extern struct CMD_TYPE_ENTREEANA *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint num );
 extern gboolean Recuperer_entreeANADB_simple ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_ENTREEANA *Recuperer_entreeANADB_simple_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_ENTREEANA *entreeana );
#endif
/*--------------------------------------------------------------------------------------------------------*/
