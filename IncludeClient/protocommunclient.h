/**********************************************************************************************************/
/* IncludeClient/protocommun_client.h:   Chargement des hearder communs aux clients                        */
/* Projet WatchDog version 1.7     Gestion d'habitat                         sam 27 sep 2010 15:08:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _PROTO_COMMUN_CLIENT_H_
 #define _PROTO_COMMUN_CLIENT_H_

 #include <gnome.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>

 #include "Reseaux.h"
 #include "Erreur.h"
 #include "trame.h"
 #include "motifs.h"
 #include "Config_cli.h"
 #include "client.h"

/******************************************** Déclaration des prototypes **********************************/

                                                                                    /* Dans mnemoniques.c */
 extern GdkColor *Couleur_bit_interne ( gint num );
 extern gchar *Type_bit_interne ( gint num );
 extern gint Type_bit_interne_int ( gchar *type );
 extern gchar *Type_bit_interne_court ( gint num );

                                                                                          /* Dans ident.c */
 extern void Envoyer_identification ( struct CLIENT *client );
 extern gint Connecter_au_serveur ( struct CLIENT *client );
 extern gboolean Envoi_serveur_reel ( struct CLIENT *client, gint tag, gint ss_tag,
                                      gchar *buffer, gint taille );

                                                                                            /* Dans ssl.c */
 extern void Close_ssl ( struct CLIENT *client );
 extern gboolean Init_ssl ( struct CLIENT *client );
 extern gboolean Connecter_ssl ( struct CLIENT *client );

#endif
/*--------------------------------------------------------------------------------------------------------*/
