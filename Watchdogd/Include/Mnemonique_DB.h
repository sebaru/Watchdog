/**********************************************************************************************************/
/* Watchdogd/Include/Mnemonique.h        Déclaration structure internes des mnemoniques watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 21 jan 2004 18:45:59 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
#ifndef _MNEMONIQUE_H_
 #define _MNEMONIQUE_H_

 #include <glib.h>
 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_mnemoniques.h"

 #define NOM_TABLE_MNEMO    "mnemos"

 struct MNEMONIQUEDB
  { guint id;                                                      /* Numero du mnemo dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1];
    gchar  objet[NBR_CARAC_OBJET_MNEMONIQUE_UTF8+1];
    gchar  acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    guint  type;                                                       /* Etat, prealarme, defaut, alarme */
    guint  num;                         /* Numéro du fichier syn correspondant(pas l'index dans la table) */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern struct MNEMONIQUEDB *Rechercher_mnemoDB ( struct LOG *log, struct DB *db, guint id );
 extern SQLHSTMT Recuperer_mnemoDB ( struct LOG *log, struct DB *db );
 extern struct MNEMONIQUEDB *Recuperer_mnemoDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern SQLHSTMT Recuperer_mnemoDB_for_courbe ( struct LOG *log, struct DB *db );
 extern struct MNEMONIQUEDB *Recuperer_mnemoDB_for_courbe_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern gint Ajouter_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MNEMONIQUE *mnemo );
 extern gboolean Retirer_mnemoDB ( struct LOG *log, struct DB *db, struct CMD_ID_MNEMONIQUE *mnemo );
 extern gboolean Modifier_mnemoDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MNEMONIQUE *mnemo );
 extern struct MNEMONIQUEDB *Rechercher_mnemoDB_type_num ( struct LOG *log, struct DB *db,
                                                         struct CMD_TYPE_NUM_MNEMONIQUE *critere );

 extern gboolean Creer_db_mnemo ( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
