/**********************************************************************************************************/
/* Watchdogd/Include/Message.h        Déclaration structure internes des messages watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _MESSAGE_DB_H_
 #define _MESSAGE_DB_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_message.h"

 #define NOM_TABLE_MSG       "msgs"

/*************************************** Définitions des prototypes ***************************************/
 extern struct CMD_TYPE_MESSAGE *Rechercher_messageDB ( struct LOG *log, struct DB *db, guint num );
 extern struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_id ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_messageDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_MESSAGE *Recuperer_messageDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_messageDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg );
 extern gboolean Retirer_messageDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg );
 extern gboolean Modifier_messageDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg );

#endif
/*--------------------------------------------------------------------------------------------------------*/
