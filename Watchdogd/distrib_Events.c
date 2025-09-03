/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                    sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Convert_libelle_dynamique: Conversion du libelle en parametre pour gérer les $ dynamiques                                  */
/* Entrée : le libelle                                                                                                        */
/* Sortie: le nouveau libelle adapté, a freer ensuite                                                                         */
/******************************************************************************************************************************/
 gchar *Convert_libelle_dynamique ( gchar *libelle_src )
  { gchar prefixe[128], tech_id[32], acronyme[64], suffixe[128], libelle[256], chaine[32];
    gint taille_result = 256;
    gchar *result = g_try_malloc0 ( taille_result );
    if (!result)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Memory error for '%s'", libelle_src );
       return(NULL);
     }
    g_snprintf ( result, taille_result, "%s", libelle_src );                                            /* Résultat potentiel */
    g_snprintf ( libelle, sizeof(libelle), "%s", libelle_src );                                           /* Copie de travail */
encore:
    memset ( prefixe,  0, sizeof(prefixe)  );                       /* Mise à zero pour gérer correctement les fins de tampon */
    memset ( suffixe,  0, sizeof(suffixe)  );
    memset ( tech_id,  0, sizeof(tech_id)  );
    memset ( acronyme, 0, sizeof(acronyme) );

    sscanf ( libelle, "%128[^$]$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", prefixe, tech_id, acronyme, suffixe );

    if (prefixe[0] == '\0')                                                        /* si pas de prefixe, on retente en direct */
     { sscanf ( libelle, "$%32[^:]:%64[a-zA-Z0-9_]%128[^\n]", tech_id, acronyme, suffixe ); }

    if (tech_id[0] != '\0' && acronyme[0] != '\0')                               /* Si on a trouvé un couple tech_id:acronyme */
     { struct DLS_REGISTRE *reg;
       struct DLS_AI *ai;
       g_snprintf( result, taille_result, "%s", prefixe );                                                        /* Prologue */
       if ( (ai = Dls_data_lookup_AI ( tech_id, acronyme )) != NULL )
        { /*if (ai->val_ech-roundf(ai->val_ech) == 0.0)
           { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
          else*/
           { g_snprintf( chaine, sizeof(chaine), "%.02f %s", ai->valeur, ai->unite ); }
        }
       else if ( (reg = Dls_data_lookup_REGISTRE ( tech_id, acronyme )) != NULL )
        { g_snprintf( chaine, sizeof(chaine), "%.02f %s", reg->valeur, reg->unite ); }
       else g_snprintf( chaine, sizeof(chaine), "bit inconnu" );
       g_strlcat ( result, chaine, taille_result );
       g_strlcat ( result, suffixe, taille_result );
       g_snprintf( libelle, sizeof(libelle), "%s", result );                              /* recopie pour prochaine itération */
       goto encore;
     }
    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Message parsé final: %s", result );
    return(result);
  }
/******************************************************************************************************************************/
/* Gerer_arrive_Axxx_dls: Gestion de l'arrive des sorties depuis DLS (Axxx = 1)                                               */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Axxx_dls ( void )
  { JsonNode *RootNode;
    gint cpt;

    cpt=0;
    while ( Partage->Liste_DO && cpt < 50 )
     { pthread_rwlock_wrlock ( &Partage->Liste_DO_synchro );
       RootNode = Partage->Liste_DO->data;                                                     /* Recuperation du numero de a */
       Partage->Liste_DO = g_slist_remove ( Partage->Liste_DO, RootNode );
       pthread_rwlock_unlock ( &Partage->Liste_DO_synchro );

       if (MSRV_Map_to_thread ( RootNode ))
        { JsonNode *Node = Json_node_create ();
          if (Node)
           { Json_node_add_string ( Node, "tech_id",  Json_get_string ( RootNode, "tech_id" ) );
             Json_node_add_string ( Node, "acronyme", Json_get_string ( RootNode, "acronyme" ) );
             Json_node_add_bool   ( Node, "etat",     Json_get_bool   ( RootNode, "etat" ) );
             MQTT_Send_to_topic ( Partage->MQTT_local_session, Node, TRUE, "SET_DO/%s/%s",
                                      Json_get_string ( RootNode, "thread_tech_id" ),
                                      Json_get_string ( RootNode, "thread_acronyme" ) );
             Json_node_unref ( Node );
           }
          else Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s:%s': Json node create error",
                         Json_get_string ( RootNode, "thread_tech_id" ),
                         Json_get_string ( RootNode, "thread_acronyme" ) );
        }
       else Info_new( __func__, Config.log_msrv, LOG_NOTICE,
                      "'%s:%s' is not mapped. dropping",
                       Json_get_string ( RootNode, "tech_id" ), Json_get_string ( RootNode, "acronyme" ) );
       Json_node_unref ( RootNode );
       cpt++;
     }

    cpt=0;
    while ( Partage->Liste_AO && cpt < 50 )
     { pthread_rwlock_wrlock( &Partage->Liste_AO_synchro );                            /* Ajout dans la liste de AO à traiter */
       RootNode = Partage->Liste_AO->data;                                                    /* Recuperation du numero de AO */
       Partage->Liste_AO = g_slist_remove ( Partage->Liste_AO, RootNode );
       pthread_rwlock_unlock( &Partage->Liste_AO_synchro );                            /* Ajout dans la liste de AO à traiter */

       if (MSRV_Map_to_thread ( RootNode ))
        { JsonNode *Node = Json_node_create ();
          if (Node)
           { Json_node_add_string ( Node, "tech_id",  Json_get_string ( RootNode, "tech_id" ) );
             Json_node_add_string ( Node, "acronyme", Json_get_string ( RootNode, "acronyme" ) );
             Json_node_add_double ( Node, "valeur",   Json_get_double ( RootNode, "valeur" ) );
             MQTT_Send_to_topic ( Partage->MQTT_local_session, Node, TRUE, "SET_AO/%s/%s",
                                      Json_get_string ( RootNode, "thread_tech_id" ),
                                      Json_get_string ( RootNode, "thread_acronyme" ) );
             Json_node_unref ( Node );
           }
          else Info_new( __func__, Config.log_msrv, LOG_ERR, "'%s:%s': Json node create error",
                         Json_get_string ( RootNode, "thread_tech_id" ),
                         Json_get_string ( RootNode, "thread_acronyme" ) );
        }
       else Info_new( __func__, Config.log_msrv, LOG_NOTICE,
                      "'%s:%s' is not mapped. dropping",
                       Json_get_string ( RootNode, "tech_id" ), Json_get_string ( RootNode, "acronyme" ) );
       Json_node_unref ( RootNode );
       cpt++;
     }

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
