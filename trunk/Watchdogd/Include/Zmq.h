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

 struct MSRV_EVENT
  { guint tag;
    gchar instance[12];
    gchar thread[12];
  };

 struct ZMQ_SET_BIT
  { gint type;
    gint num;
  };

 enum
  { TAG_ZMQ_TO_HISTO,
    TAG_ZMQ_TO_THREADS,
    TAG_ZMQ_SET_BIT,
    NBR_ZMQ_TAG
  };
/************************************************ Définitions des prototypes **************************************************/
 extern struct ZMQUEUE *New_zmq ( gint pattern, gchar *name );                                                  /* Dans zmq.c */
 extern gboolean Bind_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port );
 extern gboolean Connect_zmq ( struct ZMQUEUE *zmq, gchar *type, gchar *nom, gint port );
 extern void Close_zmq ( struct ZMQUEUE *zmq );
 extern gboolean Send_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille );
 extern gboolean Send_zmq_with_tag ( struct ZMQUEUE *zmq, gint tag, gchar *target_instance, gchar *target_thread, void *source, gint taille );
 extern gint Recv_zmq ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern gint Recv_zmq_block ( struct ZMQUEUE *zmq, void *buf, gint taille_buf );
 extern gint Recv_zmq_with_tag ( struct ZMQUEUE *zmq, void *buf, gint taille_buf, struct MSRV_EVENT **event, void **payload );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
