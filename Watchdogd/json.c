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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <glib.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Json_jwt_to_node: Prepare un RootNode depuis un token JWT avec validation                                                  */
/* Entrée: le token au format string                                                                                          */
/* Sortie: NULL si erreur, sinon le jsonnode associé                                                                          */
/******************************************************************************************************************************/
 JsonNode *Json_jwt_to_node ( gchar *source )
  { jwt_t *token;
    gchar *key = Json_get_string ( Config.config, "domain_secret" );
    if ( jwt_decode ( &token, source, key, strlen(key) ) ) return(NULL);
    gchar *RootNode_char = jwt_get_grants_json ( token, NULL );                                 /* Convert from token to Json */
    jwt_free (token);
    JsonNode *RootNode = Json_get_from_string ( RootNode_char );
    g_free(RootNode_char);
    return(RootNode);
 }
/******************************************************************************************************************************/
/* Json_node_to_jwt: Prepare un token jwt depuis un jsonnode                                                                  */
/* Entrée: le jsonnode                                                                                                        */
/* Sortie: le token, ou NULL si erreur                                                                                        */
/******************************************************************************************************************************/
 gchar *Json_node_to_jwt ( JsonNode *RootNode )
  { jwt_t *token = NULL;
    if (jwt_new (&token)) return(NULL);

    gchar *key = Json_get_string ( Config.config, "domain_secret" );
    if (jwt_set_alg ( token, jwt_str_alg ( "HS256" ), key, strlen(key) ) )
     { jwt_free (token);
       return(NULL);
     }
    gchar *rootnode_string = Json_node_to_string ( RootNode );
    jwt_add_grants_json ( token, rootnode_string );
    g_free(rootnode_string);

    gchar *token_string = jwt_encode_str (token);
    jwt_free (token);
    if (!token_string) return(NULL);
    return(token_string);
  }
/******************************************************************************************************************************/
/* Json_create: Prepare un RootNode pour creer un nouveau buffer json                                                         */
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
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_string ( JsonNode *RootNode, gchar *name, gchar *chaine )
  { JsonObject *object = json_node_get_object (RootNode);
    if (chaine) json_object_set_string_member ( object, name, chaine );
           else json_object_set_null_member   ( object, name );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_bool ( JsonNode *RootNode, gchar *name, gboolean valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_boolean_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_node_add_double: Ajoute un enregistrement name/double dans le RootNode                                                */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_double ( JsonNode *RootNode, gchar *name, gdouble valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_double_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_int ( JsonNode *RootNode, gchar *name, gint64 valeur )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_int_member ( object, name, valeur );
  }
/******************************************************************************************************************************/
/* Json_node_add_null: Ajoute un enregistrement NULL dans le RootNode                                                         */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_add_null ( JsonNode *RootNode, gchar *name )
  { JsonObject *object = json_node_get_object (RootNode);
    json_object_set_null_member   ( object, name );
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonArray *Json_node_add_array ( JsonNode *RootNode, gchar *name )
  { JsonObject *object = json_node_get_object (RootNode);
    JsonArray *tableau = json_array_new();
    json_object_set_array_member ( object, name, tableau );
    return(tableau);
  }
/******************************************************************************************************************************/
/* Json_add_string: Ajoute un enregistrement name/string dans le RootNode                                                     */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Json_node_add_objet ( JsonNode *RootNode, gchar *name )
  { JsonObject *RootObject = json_node_get_object (RootNode);
    JsonNode *new_node = json_node_alloc();
    json_node_set_object ( new_node, json_object_new() );
    json_object_set_member ( RootObject, name, new_node );
    return(new_node);
  }
/******************************************************************************************************************************/
/* Json_array_add_element: Ajoute un enregistrement dans le tableau                                                           */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_array_add_element ( JsonArray *array, JsonNode *element )
  { json_array_add_element ( array, element ); }
/******************************************************************************************************************************/
/* Json_node_foreach_array_element: Lance une fonction ne parametre sur chacun des elements d'un tableau                      */
/* Entrée: le RootNode, le nom du parametre, la valeur                                                                        */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_foreach_array_element ( JsonNode *RootNode, gchar *nom, JsonArrayForeach fonction, gpointer data )
  { json_array_foreach_element ( Json_get_array ( RootNode, nom ), fonction, data ); }
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
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(NULL); }
    return(json_object_get_string_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_double: Recupere le double dont le nom est en parametre                                                           */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gdouble Json_get_double ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(0.0); }
    return(json_object_get_double_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_bool: Recupere le booleen dont le nom est en parametre                                                            */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_get_bool ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(FALSE); }
    return(json_object_get_boolean_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_int: Recupere l'entier dont le nom est en parametre                                                               */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gint Json_get_int ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(0); }
    return(json_object_get_int_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonArray *Json_get_array ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(NULL); }
    return(json_object_get_array_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonObject *Json_get_object_as_object ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(NULL); }
    return(json_object_get_object_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_get_string: Recupere la chaine de caractere dont le nom est en parametre                                              */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 JsonNode *Json_get_object_as_node ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(NULL); }
    return(json_object_get_member ( object, chaine ));
  }
/******************************************************************************************************************************/
/* Json_has_member: Test la presence d'un membre dans le noeud                                                                */
/* Entrée: la query, le nom du parametre                                                                                      */
/* Sortie: la chaine de caractere                                                                                             */
/******************************************************************************************************************************/
 gboolean Json_has_member ( JsonNode *query, gchar *chaine )
  { JsonObject *object = json_node_get_object (query);
    if (!object) { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Object is null for '%s'", __func__, chaine );  return(FALSE); }
    return( json_object_has_member ( object, chaine ) && !json_object_get_null_member ( object, chaine ) );
  }
/******************************************************************************************************************************/
/* Json_node_unref: Libère un noeud Joson                                                                                     */
/* Entrée: le noeud json                                                                                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Json_node_unref( JsonNode *RootNode )
  { if (RootNode) json_node_unref ( RootNode ); }
/******************************************************************************************************************************/
/* Json_read_from_file: Recupere un ficher et le lit au format Json                                                           */
/* Entrée: le nom de fichier                                                                                                  */
/* Sortie: le buffer JsonNode                                                                                                 */
/******************************************************************************************************************************/
 JsonNode *Json_read_from_file ( gchar *filename )
  { struct stat stat_buf;
    if (stat ( filename, &stat_buf)==-1) return(NULL);

    gchar *content = g_try_malloc0 ( stat_buf.st_size+1 );
    if (!content) return(NULL);

    gint fd = open ( filename, O_RDONLY );
    if (!fd)
     { g_free(content);
       return(NULL);
     }

    if (read ( fd, content, stat_buf.st_size ) != stat_buf.st_size)
     { g_free(content);
       return(NULL);
     }
    close(fd);

    JsonNode *node = Json_get_from_string ( content );
    g_free(content);
    return(node);
  }
/******************************************************************************************************************************/
/* Json_write_to_file: Sauvegarde un JsonNode dans un fichier                                                                 */
/* Entrée: le nom de fichier et le buffer Json                                                                                */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Json_write_to_file ( gchar *filename, JsonNode *RootNode )
  { unlink ( filename );
    gint fd = creat ( filename, S_IWUSR | S_IRUSR );
    if (!fd)
     { Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Open %s to write Failed", __func__, filename ); return(FALSE); }
    gchar *buf=Json_node_to_string ( RootNode );
    if (!buf)
     { close(fd);
       Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Json to Buf failed, writing to %s", __func__, filename );
       return(FALSE);
     }
    gint taille = strlen(buf);
    if (write ( fd, buf, taille ) != taille)
     { close(fd);
       Info_new ( Config.log, Config.log_msrv, LOG_ERR, "%s: Error writing to %s: %s", __func__, filename, strerror(errno) );
       return(FALSE);
     }
    close(fd);
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
