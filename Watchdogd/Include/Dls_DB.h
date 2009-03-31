/**********************************************************************************************************/
/* Watchdogd/Include/Dls.h                  Définitions des constantes programme DLS                      */
/*        projet Watchdog v2.0     par LEFEVRE Sebastien                    sam 09 oct 2004 10:10:32 CEST */
/**********************************************************************************************************/

 #ifndef _DLS_DB_H_
  #define _DLS_DB_H_

 #include <glib.h>
 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_dls.h"

 #define NOM_TABLE_DLS         "dls"

 struct PLUGIN_DLS
  { gchar nom[ NBR_CARAC_PLUGIN_DLS_UTF8 + 1 ];
    guint id;
    guint on;
  };

/*************************************** Prototypes de fonctions ******************************************/
 extern gboolean Retirer_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_ID_PLUGIN_DLS *dls );
 extern gint Ajouter_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_ADD_PLUGIN_DLS *dls );
 extern gboolean Creer_db_dls ( struct LOG *log, struct DB *db );
 extern SQLHSTMT Recuperer_plugins_dlsDB( struct LOG *log, struct DB *db );
 extern struct PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern struct PLUGIN_DLS *Rechercher_plugin_dlsDB( struct LOG *log, struct DB *db, gint id );
 extern gboolean Modifier_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PLUGIN_DLS *dls );
 #endif
/*--------------------------------------------------------------------------------------------------------*/
