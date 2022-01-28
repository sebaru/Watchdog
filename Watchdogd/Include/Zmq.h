/******************************************************************************************************************************/
/* Watchdogd/Zmq.h      Déclarations générales des fonctions d'échanges intra process ou externes                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                     08.01.2018 13:23:50*/
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Zmq.h
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

 #ifndef _ZMQ_H_
 #define _ZMQ_H_

 #include <glib.h>
 #include <zmq.h>

 #define ZMQUEUE_LOCAL_MASTER    "local_msrv"
 #define ZMQUEUE_LOCAL_BUS       "local_bus"

 struct ZMQUEUE
  { void *socket;
    gint pattern;
    gchar name[32];
    gchar endpoint[32];
  };

/************************************************ Définitions des prototypes **************************************************/
 extern struct ZMQUEUE *Zmq_Connect ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port );     /* Dans zmq.c */
 extern struct ZMQUEUE *Zmq_Bind ( gint pattern, gchar *name, gchar *type, gchar *endpoint, gint port );
 extern void Zmq_Close ( struct ZMQUEUE *zmq );
 extern gboolean Zmq_Send_as_raw ( struct ZMQUEUE *zmq, void *buf, gint taille );
 extern gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern void Zmq_Send_DI_to_master_new ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme, gboolean etat );
 extern void Zmq_Send_AI_to_master ( struct SUBPROCESS *module, JsonNode *ai);
 extern void Zmq_Send_AI_to_master_new ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme, gdouble valeur, gboolean in_range);
 extern void Zmq_Send_CDE_to_master_new ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme );
 extern void Zmq_Send_WATCHDOG_to_master_new ( struct SUBPROCESS *module, gchar *tech_id, gchar *acronyme, gint consigne );
 extern JsonNode *Recv_zmq_with_json ( struct ZMQUEUE *zmq, const gchar *my_tech_id, gchar *buf, gint taille_buf );
 extern gboolean Zmq_Send_json_node ( struct ZMQUEUE *zmq, const gchar *zmq_src_tech_id, const gchar *zmq_dst_tech_id,
                                      JsonNode *RootNode );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
