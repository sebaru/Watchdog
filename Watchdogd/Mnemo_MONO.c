/******************************************************************************************************************************/
/* Watchdogd/Mnemo_MONO.c        Déclaration des fonctions pour la gestion des booleans                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    24.06.2019 22:07:06 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_MONO.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Mnemo_auto_create_MONO: Ajoute un mnemonique dans la base via le tech_id                                                   */
/* Entrée: le tech_id, l'acronyme, le libelle                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_MONO ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  { gchar *acro, *libelle;
    gboolean retour;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    retour = SQL_Write_new ( "INSERT INTO mnemos_MONO SET deletable='%d', tech_id='%s',acronyme='%s',libelle='%s' "
                             "ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                             deletable, tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    Dls_data_MONO_lookup ( tech_id, acronyme );                                    /* Recherche ou Création du message en RAM */

    return (retour);
  }
/******************************************************************************************************************************/
/* Charger_confDB_un_MONO: Recupération de la conf d'un monostable                                                            */
/* Entrée: néant                                                                                                              */
/* Sortie: le message est chargé en mémoire                                                                                   */
/******************************************************************************************************************************/
 static void Charger_confDB_un_MONO (JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { gint  *cpt_p    = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    gboolean etat   = Json_get_bool   ( element, "etat" );
    (*cpt_p)++;
    struct DLS_MONO *mono = Dls_data_MONO_lookup ( tech_id, acronyme );            /* Recherche ou Création du message en RAM */
    if (mono) /* A l'init, on recopie tous les champs */
     { mono->etat   = mono->next_etat = etat; }
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: MONO '%s:%s'=%d loaded", __func__, tech_id, acronyme, etat );
  }
/******************************************************************************************************************************/
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_MONO ( void )
  { gint cpt = 0;

    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { SQL_Select_to_json_node ( RootNode, "monos", "SELECT m.tech_id, m.acronyme, m.etat FROM mnemos_MONO as m" );
       Json_node_foreach_array_element ( RootNode, "monos", Charger_confDB_un_MONO, &cpt );
       Json_node_unref ( RootNode );
     } else Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Memory Error", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d MONO loaded", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_MONO_to_json : Formate un bit au format JSON                                                                           */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_MONO_to_json ( JsonNode *element, struct DLS_MONO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_all_MONO_to_json: Transforme tous les bits en JSON                                                                     */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_MONO_to_json ( JsonNode *target )
  { gint cpt = 0;

    JsonArray *RootArray = Json_node_add_array ( target, "mnemos_MONO" );
    GSList *liste = Partage->Dls_data_MONO;
    while ( liste )
     { struct DLS_MONO *bit = (struct DLS_MONO *)liste->data;
       JsonNode *element = Json_node_create();
       Dls_MONO_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
       cpt++;
     }
    Json_node_add_int ( target, "nbr_mnemos_MONO", cpt );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
