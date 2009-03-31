/**********************************************************************************************************/
/* Watchdog/Db/Utilisateur/proto_util.h    Header locaux                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 21 avr 2003 13:09:57 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include "Utilisateur_DB.h"

 extern gchar *Groupes_vers_string ( guint *source );
 extern gint Get_login_failed( struct LOG *log, struct DB *db, guint id );
 extern gboolean Ajouter_one_login_failed( struct LOG *log, struct DB *db, guint id, gint max_login_failed );
 extern gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable );
 extern guint *String_vers_groupes ( gchar *source );
 extern gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable );

/*--------------------------------------------------------------------------------------------------------*/
