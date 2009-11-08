/**********************************************************************************************************/
/* Watchdogd/Include/Icones.h        Déclaration structure internes des icones watchdog                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:33:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ICONES_H_
 #define _ICONES_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_icone.h"

 #define NOM_TABLE_CLASSE     "class"
 #define NOM_TABLE_ICONE      "icons"

 struct CLASSEDB
  { gchar libelle[NBR_CARAC_CLASSE_ICONE_UTF8+1];
    guint id;
  };

 struct ICONEDB
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[NBR_CARAC_LIBELLE_ICONE_UTF8+1];
    guint id_classe;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern struct ICONEDB *Rechercher_iconeDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_iconeDB ( struct LOG *log, struct DB *db, guint classe );
 extern struct ICONEDB *Recuperer_iconeDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_iconeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone );
 extern gboolean Retirer_iconeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone );
 extern gboolean Modifier_iconeDB( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone );

 extern struct CLASSEDB *Rechercher_classeDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_classeDB ( struct LOG *log, struct DB *db );
 extern struct CLASSEDB *Recuperer_classeDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_classeDB ( struct LOG *log, struct DB *db, struct CMD_ADD_CLASSE *classe );
 extern gboolean Retirer_classeDB ( struct LOG *log, struct DB *db, struct CMD_ID_CLASSE *classe );
 extern gboolean Modifier_classeDB( struct LOG *log, struct DB *db, struct CMD_EDIT_CLASSE *classe );
#endif
/*--------------------------------------------------------------------------------------------------------*/
