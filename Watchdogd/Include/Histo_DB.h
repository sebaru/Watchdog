/**********************************************************************************************************/
/* Watchdogd/Include/Histo_DB.h        Déclaration structure internes des historiques watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _HISTO_H_
 #define _HISTO_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Utilisateur_DB.h"
 #include "Message_DB.h"

 #define NOM_TABLE_HISTO       "histo"
 #define NOM_TABLE_HISTO_HARD  "histo_hard"

 struct HISTODB
  { struct MSGDB msg;
    gchar nom_ack [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    guint date_create_sec;
    guint date_create_usec;
    time_t date_fixe;
  };

 struct HISTO_HARDDB
  { struct HISTODB histo;
    time_t date_fin;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Clear_histoDB ( void );
 extern gboolean Retirer_histoDB ( struct LOG *log, struct DB *db, struct CMD_ID_HISTO *histo );
 extern gboolean Ajouter_histoDB ( struct LOG *log, struct DB *db, struct HISTODB *histo );
 extern gboolean Recuperer_histoDB ( struct LOG *log, struct DB *db );
 extern struct HISTODB *Recuperer_histoDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_histoDB ( struct LOG *log, struct DB *db, struct CMD_EDIT_HISTO *histo );
 extern struct HISTODB *Rechercher_histoDB( struct LOG *log, struct DB *db, gint id );

 extern gboolean Ajouter_histo_hardDB ( struct LOG *log, struct DB *db, struct HISTO_HARDDB *histo );
 extern gboolean Creer_db_histo_hard ( struct LOG *log, struct DB *db );
 extern  SQLHSTMT Rechercher_histo_hardDB ( struct LOG *log, struct DB *db,
                                           struct CMD_REQUETE_HISTO_HARD *critere );
 extern struct HISTO_HARDDB *Rechercher_histo_hardDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );

#endif
/*--------------------------------------------------------------------------------------------------------*/
