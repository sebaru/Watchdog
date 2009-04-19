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

 struct MSGDB
  { guint id;
    guint  num;                                                    /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    guint  type;                                                       /* Etat, prealarme, defaut, alarme */
    guint  num_syn;                     /* Numéro du fichier syn correspondant(pas l'index dans la table) */
    guint  num_voc;                                                    /* Numero du fichier vocal associé */
    gboolean not_inhibe;                          /* Flag pour la gestion par exemple de l'inhibition ... */
    gboolean sms;                                                                /* Envoi de sms ou non ? */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern struct MSGDB *Rechercher_messageDB ( struct LOG *log, struct DB *db, guint num );
 extern struct MSGDB *Rechercher_messageDB_par_id ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_messageDB ( struct LOG *log, struct DB *db );
 extern struct MSGDB *Recuperer_messageDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_messageDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MESSAGE *msg );
 extern gboolean Retirer_messageDB ( struct LOG *log, struct DB *db, struct CMD_ID_MESSAGE *msg );
 extern gboolean Modifier_messageDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MESSAGE *msg );

#endif
/*--------------------------------------------------------------------------------------------------------*/
