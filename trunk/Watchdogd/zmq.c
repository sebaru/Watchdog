/******************************************************************************************************************************/
/* Watchdogd/zmq.c        Gestion des echanges ZMQ                                                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    06.01.2018 11:42:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * zmq.c
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
/* Zmq_subscribe: Souscris au topic en parametre                                                                              */
/* Entrée: la queue, le topic                                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gboolean Zmq_subscribe ( struct ZMQUEUE *zmq )
  { if ( zmq_setsockopt ( zmq->socket, ZMQ_SUBSCRIBE, NULL, 0 ) == -1 )                          /* Subscribe to all messages */
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: ZMQ subscript to all for '%s' failed (%s)",
                 __func__, zmq->name, zmq_strerror(errno) );
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: ZMQ subscribe for '%s' OK", __func__, zmq->name );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Zmq_Bind: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le endpoint et le port                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 struct ZMQUEUE *Zmq_Bind ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port )
  { struct ZMQUEUE *zmq;
    zmq = (struct ZMQUEUE *)g_try_malloc0( sizeof(struct ZMQUEUE) );
    if (!zmq)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: New ZMQ Socket '%s' Failed. Memory Error (%s)",
                 __func__, name, zmq_strerror(errno) );
       return(NULL);
     }

    zmq->pattern = pattern;
    g_snprintf( zmq->name, sizeof(zmq->name), "%s", name );
    if ( (zmq->socket = zmq_socket ( Partage->zmq_ctx, pattern )) == NULL)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                 "%s: New ZMQ Socket '%s' Failed (%s)", __func__, name, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }

    if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, endpoint, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, endpoint );
    if ( zmq_bind (zmq->socket, zmq->endpoint) == -1 )
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                 "%s: ZMQ Bind '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }
    else Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: ZMQ Bind '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) Zmq_subscribe(zmq);
    return(zmq);
  }
/******************************************************************************************************************************/
/* Zmq_Bind: Bind la socket en parametre                                                                                      */
/* Entrée: le type, le endpoint et le port                                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 struct ZMQUEUE *Zmq_Connect ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port )
  { struct ZMQUEUE *zmq;
    zmq = (struct ZMQUEUE *)g_try_malloc0( sizeof(struct ZMQUEUE) );
    if (!zmq)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: New ZMQ Socket '%s' Failed. Memory Error (%s)",
                 __func__, name, zmq_strerror(errno) );
       return(NULL);
     }

    zmq->pattern = pattern;
    g_snprintf( zmq->name, sizeof(zmq->name), "%s", name );
    if ( (zmq->socket = zmq_socket ( Partage->zmq_ctx, pattern )) == NULL)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                 "%s: New ZMQ Socket '%s' Failed (%s)", __func__, name, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }

    if (port) g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s:%d", type, endpoint, port );
         else g_snprintf( zmq->endpoint, sizeof(zmq->endpoint), "%s://%s", type, endpoint );
    if ( zmq_connect (zmq->socket, zmq->endpoint) == -1 )
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                 "%s: ZMQ Connect '%s' to '%s' Failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       g_free(zmq);
       return(NULL);
     }
    else Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: ZMQ Connect '%s' to '%s' OK", __func__, zmq->name, zmq->endpoint );
    if (zmq->pattern == ZMQ_SUB) Zmq_subscribe(zmq);
    return(zmq);
  }
/******************************************************************************************************************************/
/* Zmq_Close: Ferme une socket ZMQ                                                                                            */
/* Entrée: la queue                                                                                                           */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Zmq_Close ( struct ZMQUEUE *zmq )
  { if (!zmq) return;
    Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: ZMQ closing '%s'", __func__, zmq->name );
    zmq_close ( zmq->socket );
    g_free(zmq);
  }
/******************************************************************************************************************************/
/* Send_zmq: Envoie un message dans la socket                                                                                 */
/* Entrée: la socket, le message, sa longueur                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Zmq_Send_as_raw ( struct ZMQUEUE *zmq, void *buf, gint taille )
  { if (!zmq) return(FALSE);
    if (zmq_send( zmq->socket, buf, taille, 0 ) == -1)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                "%s: Send to ZMQ '%s' ('%s') failed (%s)", __func__, zmq->name, zmq->endpoint, zmq_strerror(errno) );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_zmq, LOG_DEBUG,
             "%s: Send to ZMQ '%s' ('%s') %d bytes", __func__, zmq->name, zmq->endpoint, taille );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Zmq_Send_with_tag: Envoie un message dans la socket avec le tag en prefixe                                                 */
/* Entrée: la socket, le tag, le message, sa longueur                                                                         */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Zmq_Send_json_node ( struct ZMQUEUE *zmq, const gchar *zmq_src_tech_id,
                               const gchar *zmq_dst_tech_id, const gchar *zmq_tag, JsonNode *RootNode )
  { gboolean own_node=FALSE;
    if (!zmq) return(FALSE);

    if (!RootNode) { RootNode = Json_node_create(); own_node = TRUE; }

    Json_node_add_string ( RootNode, "zmq_src_tech_id", zmq_src_tech_id );
    Json_node_add_string ( RootNode, "zmq_tag", zmq_tag );

    if (!zmq_dst_tech_id)  zmq_dst_tech_id  ="*";
    Json_node_add_string ( RootNode, "zmq_dst_tech_id", zmq_dst_tech_id );

    Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: '%s' ('%s') : SENDING %s -> %s/%s", __func__,
              zmq->name, zmq->endpoint, zmq_src_tech_id, zmq_dst_tech_id, zmq_tag );

    gboolean retour;
    gchar *buf = Json_node_to_string ( RootNode );
    if (own_node) json_node_unref(RootNode);
    retour = Zmq_Send_as_raw( zmq, buf, strlen(buf) );
    g_free(buf);

    if (retour==FALSE)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR,
                "%s: '%s' ('%s') : ERROR SENDING %s -> %s/%s", __func__, zmq->name, zmq->endpoint,
                 zmq_src_tech_id, zmq_dst_tech_id, zmq_tag );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf, ZMQ_DONTWAIT );
    return(byte);
  }
/******************************************************************************************************************************/
/* Recv_zmq: Receptionne un message sur le file en paremetre (sans attendre)                                                  */
/* Entrée: la file, le buffer d'accueil, la taille du buffer                                                                  */
/* Sortie: Nombre de caractere lu, -1 si erreur                                                                               */
/******************************************************************************************************************************/
 JsonNode *Recv_zmq_with_json ( struct ZMQUEUE *zmq, const gchar *my_tech_id, gchar *buf, gint taille_buf )
  { gint byte;
    byte = zmq_recv ( zmq->socket, buf, taille_buf, ZMQ_DONTWAIT );
    if (byte<0) return(NULL);
    if (byte>=taille_buf)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: Received %d bytes. Message too long. Dropping.", __func__, byte );
       return(NULL);
     }
    buf[byte]=0;                                                                                     /* Caractere nul d'arret */
    JsonNode *request = Json_get_from_string ( buf );
    if (!request)
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: Received %d bytes but this is not JSON", __func__, byte );
       return(NULL);
     }

    if (!Json_has_member( request, "zmq_src_tech_id"))
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: No 'zmq_src_tech_id'. Dropping.", __func__ );
       json_node_unref(request);
       return(NULL);
     }

    if (!Json_has_member( request, "zmq_dst_tech_id"))
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: No 'zmq_dst_tech_id'. Dropping.", __func__ );
       json_node_unref(request);
       return(NULL);
     }

    if (!Json_has_member( request, "zmq_tag"))
     { Info_new( Config.log, Config.log_zmq, LOG_ERR, "%s: No 'zmq_tag'. Dropping.", __func__ );
       json_node_unref(request);
       return(NULL);
     }

    gchar *zmq_dst_tech_id = Json_get_string(request,"zmq_dst_tech_id");
    gchar *zmq_tag         = Json_get_string(request,"zmq_tag");

    Info_new( Config.log, Config.log_zmq, LOG_DEBUG,
              "%s: '%s' ('%s') : %s -> %s/%s", __func__, zmq->name, zmq->endpoint,
              Json_get_string(request,"zmq_src_tech_id"), zmq_dst_tech_id, zmq_tag );

    if ( strcasecmp( zmq_dst_tech_id, "*" ) && my_tech_id && strcasecmp ( zmq_dst_tech_id, my_tech_id ) )
     { Info_new( Config.log, Config.log_zmq, LOG_DEBUG, "%s: Pas pour nous, pour '%s'. Dropping",
                 __func__, zmq_dst_tech_id );
       json_node_unref(request);
       return(NULL);
     }

    return(request);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Zmq_Send_DI_to_master ( void *zmq, gchar *my_tech_id, gchar *tech_id, gchar *acronyme, gboolean etat )
  { if (!zmq) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Json_node_add_bool   ( body, "etat", etat );
    Zmq_Send_json_node ( zmq, my_tech_id, "msrv", "SET_DI", body );
    json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Zmq_Send_AI_to_master ( void *zmq, gchar *my_tech_id, gchar *tech_id, gchar *acronyme, gfloat valeur, gboolean in_range)
  { if (!zmq) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Json_node_add_double ( body, "valeur", valeur );
    Json_node_add_bool   ( body, "in_range", in_range );
    Zmq_Send_json_node ( zmq, my_tech_id, "msrv", "SET_AI", body );
    json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Zmq_Send_CDE_to_master ( void *zmq, gchar *my_tech_id, gchar *tech_id, gchar *acronyme )
  { if (!zmq) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Zmq_Send_json_node ( zmq, my_tech_id, "msrv", "SET_CDE", body );
    json_node_unref(body);
  }
/******************************************************************************************************************************/
/* Smsg_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                         */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Zmq_Send_WATCHDOG_to_master ( void *zmq, gchar *my_tech_id, gchar *tech_id, gchar *acronyme, gint consigne )
  { if (!zmq) return;
    JsonNode *body = Json_node_create ();
    if(!body) return;
    Json_node_add_string ( body, "tech_id",  tech_id );
    Json_node_add_string ( body, "acronyme", acronyme );
    Json_node_add_int    ( body, "consigne", consigne );
    Zmq_Send_json_node ( zmq, my_tech_id, "msrv", "SET_WATCHDOG", body );
    json_node_unref(body);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
