/**********************************************************************************************************/
/* Watchdogd/Include/Camera.h        Déclaration structure internes des Camera watchdog                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:13:20 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CAMERA_DB_H_
 #define _CAMERA_DB_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_CAMERA       "cameras"

/*************************************** Définitions des prototypes ***************************************/
 extern struct CMD_TYPE_CAMERA *Rechercher_cameraDB ( struct LOG *log, struct DB *db, guint num );
 extern struct CMD_TYPE_CAMERA *Rechercher_cameraDB_par_id ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_cameraDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *msg );

#endif
/*--------------------------------------------------------------------------------------------------------*/
