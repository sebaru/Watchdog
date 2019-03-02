/******************************************************************************************************************************/
/* Watchdogd/Zmq.h      Déclarations générales des fonctions d'échanges intra process ou externes                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                     08.01.2018 13:23:50*/
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Zmq.h
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
 
 #ifndef _ZMQ_H_
 #define _ZMQ_H_

 #include <glib.h>
 #include <zmq.h>

 #define ZMQUEUE_LIVE_MSGS     "live-msgs"
 #define ZMQUEUE_LIVE_MOTIFS   "live-motifs"
 #define ZMQUEUE_LIVE_THREADS  "live-threads"
 #define ZMQUEUE_LIVE_MASTER   "live-master"

 struct ZMQUEUE
  { void *socket;
    gint pattern;
    gchar name[32];
    gchar endpoint[32];
  };

 struct ZMQ_TARGET
  { gint8 tag;
    gchar src_instance[24];
    gchar src_thread[12];
    gchar dst_instance[24];
    gchar dst_thread[12];
  };

 struct ZMQ_SET_BIT
  { gint type;
    gint num;
    gchar acronyme [ NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1 ];
    gchar dls_tech_id [ NBR_CARAC_PLUGIN_DLS_TECHID ];
  };

 enum
  { TAG_ZMQ_TO_HISTO,
    TAG_ZMQ_TO_THREADS,
    TAG_ZMQ_SET_BIT,
    TAG_ZMQ_SET_SYN_VARS,
    TAG_ZMQ_CLI,
    TAG_ZMQ_CLI_RESPONSE,
    TAG_ZMQ_SLAVE_PING,
    TAG_ZMQ_AUDIO_PLAY_WAV,
    TAG_ZMQ_AUDIO_PLAY_GOOGLE,
    NBR_ZMQ_TAG
  };
/************************************************ Définitions des prototypes **************************************************/
 extern struct ZMQUEUE *New_zmq ( gint pattern, gchar *name );                                                  /* Dans zmq.c */
 extern gboolean Bind_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port );
 extern gboolean Connect_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port );
 extern void Close_zmq ( struct ZMQUEUE *zmq );
 extern gboolean Send_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille );
 extern gboolean Send_zmq_with_tag ( struct ZMQUEUE *zmq, gint tag,
                                     const gchar *source_instance, const gchar *source_thread,
                                     const gchar *target_instance, const gchar *target_thread,
                                     void *source, gint taille );
 extern gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern gint Recv_zmq_block ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern gint Recv_zmq_with_tag ( struct ZMQUEUE *zmq, void *buf, gint taille_buf, struct ZMQ_TARGET **event, void **payload );
 extern gboolean Zmq_instance_is_target ( struct ZMQ_TARGET *event );
 extern gboolean Zmq_other_is_target ( struct ZMQ_TARGET *event );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
