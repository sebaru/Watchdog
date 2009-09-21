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

 struct CAMERADB
  { guint   id;
    gchar   libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                           /* Libelle de la camera */
    gchar   location[NBR_CARAC_LOCATION_CAMERA_UTF8];                             /* Libelle de la camera */
    gint    type;                                                            /* petite, moyenne, grande ? */
    gint    num;                                                                   /* Numéro de la caméra */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern struct CAMERADB *Rechercher_cameraDB ( struct LOG *log, struct DB *db, guint num );
 extern struct CAMERADB *Rechercher_cameraDB_par_id ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db );
 extern struct CAMERADB *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *msg );
 extern gboolean Retirer_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *msg );
 extern gboolean Modifier_cameraDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *msg );

#endif
/*--------------------------------------------------------------------------------------------------------*/
