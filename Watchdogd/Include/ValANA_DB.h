/**********************************************************************************************************/
/* Watchdogd/Include/ValANA.h        D�claration structure internes des valeurs entree analogiques        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 ao� 2005 17:09:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _VALANA_H_
 #define _VALANA_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_VALANA    "valana"

 struct VALANADB
  { guint  date;                                                                      /* Date de la photo */
    guint  num;                                            /* Numero de l'entr�e analogique photographi�e */
    guint  valeur;                                                       /* Valeur de l'entr�e analogique */
  };

/*************************************** D�finitions des prototypes ***************************************/
 extern void Ajouter_valANADB ( struct LOG *log, struct DB *db, struct VALANADB *val );
 extern SQLHSTMT Recuperer_valANADB ( struct LOG *log, struct DB *db, guint id );
 extern struct VALANADB *Recuperer_valANADB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );

 extern gboolean Creer_db_valANA ( struct LOG *log, struct DB *db );
#endif
/*--------------------------------------------------------------------------------------------------------*/
