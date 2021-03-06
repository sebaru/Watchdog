/******************************************************************************************************************************/
/* Watchdogd/json.c        Fonctions helper pour la manipulation des payload au format JSON                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    27.06.2019 09:38:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * json.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Json_create: Prepare un builder pour creer un nouveau buffer json                                                          */
/* Entrée: néant                                                                                                              */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 JsonNode *Json_node_create ( void )
  { JsonNode *RootNode;
    RootNode = json_node_alloc();
    json_node_take_object ( RootNode, json_object_new() );
    return(RootNode);
  }
/******************************************************************************************************************************/
/* Json_create: Prepare un builder pour creer un nouveau buffer json                                                          */
/* Entrée: néant                                                                                                              */
/* Sortie: NULL si erreur                                                                                                     */
/******************************************************************************************************************************/
 JsonBuilder *Json_create ( void )
  { JsonBuilder *builder;
    builder = json_builder_new();
    json_builder_begin_object(builder);
    return(builder);
  }
/******************************************************************************************************************************/
/* Json_end: Termine le builder et retourne le node associé                                                                   */
/* Entrée: le builder                                                                                                         */
/* Sortie: le rootnode associé                                                                                                */
/******************************************************************************************************************************/
 JsonNode *Json_end ( JsonBuilder *builder )
  { json_builder_end_object(builder);
    JsonNode *RootNode = json_builder_get_root(builder);
    g_object_unref(builder);
    return(RootNode);
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_string ( JsonBuilder *builder, gchar *name, gchar *chaine )
  { json_builder_set_member_name  ( builder, name );
    if (chaine) json_builder_add_string_value ( builder, chaine );
           else json_builder_add_null_value ( builder );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_string ( JsonNode *RootNode, gchar *name, gchar *chaine )
  { JsonObject *object = json_node_get_object (RootNode);
    if (chaine) json_object_set_string_member ( object, name, chaine );
           else json_object_set_null_member   ( object, name );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_bool ( JsonNode *RootNode, gchar *name, gboolean valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_boolean_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_double ( JsonNode *RootNode, gchar *name, gdouble valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_double_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_int ( JsonNode *RootNode, gchar *name, gint64 valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_int_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_node ( JsonBuilder *builder, gchar *name, JsonNode *node )
  { if (name) json_builder_set_member_name  ( builder, name );
    json_builder_add_value ( builder, node );
  }
/******************************************************************************************************************************/
/* Json_add_boolean: Ajoute un enregistrement name/bool dans le builder                                                       */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_bool ( JsonBuilder *builder, gchar *name, gboolean bool )
  { json_builder_set_member_name  ( builder, name );
    json_builder_add_boolean_value ( builder, bool );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_int ( JsonBuilder *builder, gchar *name, gint valeur )
  { json_builder_set_member_name  ( builder, name );
    json_builder_add_int_value ( builder, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_double: Ajoute un enregistrement name/double dans le builder                                                      */
/* Entrée: le builder, le nom du parametre, la valeur                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_double ( JsonBuilder *builder, gchar *name, gdouble valeur )
  { json_builder_set_member_name  ( builder, name );
    json_builder_add_double_value ( builder, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_object: Ajoute un enregistrement de type object dans le builder                                                   */
/* Entrée: le builder, le nom du parametre                                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_array ( JsonBuilder *builder, gchar *name )
  { json_builder_set_member_name  ( builder, name );
    json_builder_begin_array (builder);                                                                  /* Contenu du Status */
  }
/******************************************************************************************************************************/
/* Json_end_object: Termine l'object en cours de ocnstruction dans le builder                                                 */
/* Entrée: le builder                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_end_array ( JsonBuilder *builder )
  { json_builder_end_array (builder);                                                                    /* Contenu du Status */
  }
/******************************************************************************************************************************/
/* Json_add_object: Ajoute un enregistrement de type object dans le builder                                                   */
/* Entrée: le builder, le nom du parametre                                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_add_object ( JsonBuilder *builder, gchar *name )
  { if (name) json_builder_set_member_name  ( builder, name );
    json_builder_begin_object (builder);                                                                 /* Contenu du Status */
  }
/******************************************************************************************************************************/
/* Json_end_object: Termine l'object en cours de ocnstruction dans le builder                                                 */
/* Entrée: le builder                                                                                                         */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_end_object ( JsonBuilder *builder )
  { json_builder_end_object (builder);                                                                 /* Contenu du Status */
  }
/******************************************************************************************************************************/
/* Json_get_buf: Termine le buffer JSON en le formatant en chaine de caractere                                                */
/* Entrée: le builder, un gsize * pour récupérer la taille du buffer                                                          */
/* Sortie: le buffer, a g_freé quand plus besoin                                                                              */
/******************************************************************************************************************************/
 gchar *Json_get_buf ( JsonBuilder *builder, gsize *taille_buf_p )
  { JsonGenerator *gen;
    JsonNode *RootNode;
    gchar *result;
    gen = json_generator_new ();
    json_builder_end_object(builder);
    RootNode = json_builder_get_root(builder);
    json_generator_set_root ( gen, RootNode );
    json_node_unref(RootNode);
    json_generator_set_pretty ( gen, TRUE );
    result = json_generator_to_data (gen, taille_buf_p);
    g_object_unref(builder);
    g_object_unref(gen);
    return(result);
  }
/******************************************************************************************************************************/
/* Json_node_to_string: transforme un JsonNode en string                                                                      */
/* Entrée: le JsonNode a convertir                                                                                            */
/* Sortie: un nouveau buffer                                                                                                  */
/******************************************************************************************************************************/
 gchar *Json_node_to_string ( JsonNode *RootNode )
  { return ( json_to_string ( RootNode, TRUE ) );
  }
/******************************************************************************************************************************/
/* Json_get_from_stirng: Recupere l'object de plus haut niveau dans une chaine JSON                                           */
/* Entrée: la chaine de caractere                                                                                             */
/* Sortie: l'objet                                                                                                            */
/******************************************************************************************************************************/
 JsonNode *Json_get_from_string ( gchar *chaine )
  { return(json_from_string ( chaine, NULL )); }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gchar *Json_get_string ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return(json_object_get_string_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gfloat Json_get_float ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return((gfloat)json_object_get_double_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_get_bool ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return(json_object_get_boolean_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_int: Recupere l'entier dont le nom est en parametre                                                               */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gint Json_get_int ( JsonNode *query, gchar *chaine )
  { GValue valeur = G_VALUE_INIT;
    gint retour;
    JsonObject *object = json_node_get_object (query);
    JsonNode *node = json_object_get_member ( object, chaine );
    if (!node) { return(-1); }
    json_node_get_value ( node, &valeur );
         if ( G_VALUE_HOLDS_STRING (&valeur) ) { retour = atoi(g_value_get_string (&valeur)); }
    else if ( G_VALUE_HOLDS_INT    (&valeur) ) { retour = g_value_get_int (&valeur); }
    else if ( G_VALUE_HOLDS_INT64  (&valeur) ) { retour = g_value_get_int64 (&valeur); }
    else { printf("%s: Erreur getting '%s'\n", __func__, chaine ); retour = -1; }
    g_value_unset ( &valeur );
    return(retour);
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonArray *Json_get_array ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return(json_object_get_array_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonObject *Json_get_object_as_object ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return(json_object_get_object_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonNode *Json_get_object_as_node ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return(json_object_get_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_int: Recupere l'entier dont le nom est en parametre                                                               */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_has_member ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    return( json_object_has_member ( object, chaine ) && !json_object_get_null_member ( object, chaine ) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
