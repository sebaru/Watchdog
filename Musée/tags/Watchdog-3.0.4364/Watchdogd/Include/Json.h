/******************************************************************************************************************************/
/* Watchdogd/Json.h      Déclarations générales des fonctions de manipulation JSON                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    27.06.2019 09:43:35 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Json.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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

/************************************************ Définitions des prototypes **************************************************/
 extern JsonBuilder *Json_create ( void );
 extern void Json_add_string ( JsonBuilder *builder, gchar *name, gchar *chaine );
 extern void Json_add_int ( JsonBuilder *builder, gchar *name, gint valeur );
 extern void Json_add_double ( JsonBuilder *builder, gchar *name, gfloat valeur );
 extern void Json_add_bool ( JsonBuilder *builder, gchar *name, gboolean bool );
 extern gchar *Json_get_buf ( JsonBuilder *builder, gsize *taille_buf_p );
 extern JsonNode *Json_get_from_string ( gchar *chaine );
 extern gchar *Json_get_string ( JsonNode *query, gchar *chaine );
 extern gfloat Json_get_float ( JsonNode *query, gchar *chaine );
 extern gboolean Json_get_bool ( JsonNode *query, gchar *chaine );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
