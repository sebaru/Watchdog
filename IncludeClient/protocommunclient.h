/**********************************************************************************************************/
/* IncludeClient/protocommun_client.h:   Chargement des hearder communs aux clients                        */
/* Projet WatchDog version 1.7     Gestion d'habitat                         sam 27 sep 2010 15:08:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _PROTO_COMMUN_CLIENT_H_
 #define _PROTO_COMMUN_CLIENT_H_

 #include <gnome.h>

 #include "trame.h"
 #include "motifs.h"
 #include "Config_cli.h"

/******************************************** Déclaration des prototypes **********************************/

 extern GdkColor *Couleur_bit_interne ( gint num );
 extern gchar *Type_bit_interne ( gint num );
 extern gint Type_bit_interne_int ( gchar *type );
 extern gchar *Type_bit_interne_court ( gint num );

#endif
/*--------------------------------------------------------------------------------------------------------*/
