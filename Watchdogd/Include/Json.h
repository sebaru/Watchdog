/******************************************************************************************************************************/
/* Watchdogd/Json.h      Déclarations générales des fonctions de manipulation JSON                                            */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                27.06.2019 09:43:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Json.h
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

 #ifndef _JSON_H_
 #define _JSON_H_

 #include <json-glib/json-glib.h>
 #include <jwt.h>

/************************************************ Définitions des prototypes **************************************************/
 extern JsonNode *Json_node_create ( void );
 extern void Json_node_unref( JsonNode *RootNode );
 extern void Json_node_add_string ( JsonNode *RootNode, gchar *name, gchar *chaine );
 extern void Json_node_add_bool ( JsonNode *RootNode, gchar *name, gboolean valeur );
 extern void Json_node_add_int ( JsonNode *RootNode, gchar *name, gint64 valeur );
 extern void Json_node_add_double ( JsonNode *RootNode, gchar *name, gdouble valeur );
 extern void Json_node_add_null ( JsonNode *RootNode, gchar *name );
 extern JsonArray *Json_node_add_array ( JsonNode *RootNode, gchar *name );
 extern JsonNode *Json_node_add_objet ( JsonNode *RootNode, gchar *name );
 extern void Json_array_add_element ( JsonArray *array, JsonNode *element );
 extern void Json_node_foreach_array_element ( JsonNode *RootNode, gchar *nom, JsonArrayForeach fonction, gpointer data );
 extern gchar *Json_node_to_string ( JsonNode *RootNode );
 extern JsonNode *Json_get_from_string ( gchar *chaine );
 extern gchar *Json_get_string ( JsonNode *query, gchar *chaine );
 extern gdouble Json_get_double ( JsonNode *query, gchar *chaine );
 extern gboolean Json_get_bool ( JsonNode *query, gchar *chaine );
 extern gint Json_get_int ( JsonNode *query, gchar *chaine );
 extern JsonArray *Json_get_array ( JsonNode *query, gchar *chaine );
 extern JsonObject *Json_get_object_as_object ( JsonNode *query, gchar *chaine );
 extern JsonNode *Json_get_object_as_node ( JsonNode *query, gchar *chaine );
 extern gboolean Json_has_member ( JsonNode *query, gchar *chaine );
 extern JsonNode *Json_read_from_file ( gchar *filename );
 extern gboolean Json_write_to_file ( gchar *filename, JsonNode *RootNode );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
