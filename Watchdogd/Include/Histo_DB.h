/**********************************************************************************************************/
/* Watchdogd/Include/Histo_DB.h        D�claration structure internes des historiques watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 10 jun 2003 12:12:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Histo_DB.h
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
 
#ifndef _HISTO_H_
 #define _HISTO_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Utilisateur_DB.h"
 #include "Message_DB.h"

 #define NOM_TABLE_HISTO       "histo"
 #define NOM_TABLE_HISTO_HARD  "histo_hard"

 struct HISTODB
  { struct CMD_TYPE_MESSAGE msg;
    gchar nom_ack [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    guint date_create_sec;
    guint date_create_usec;
    time_t date_fixe;
  };

 struct HISTO_HARDDB
  { struct HISTODB histo;
    time_t date_fin;
  };

/*************************************** D�finitions des prototypes ***************************************/
 extern void Clear_histoDB ( void );
 extern gboolean Retirer_histoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_HISTO *histo );
 extern gboolean Ajouter_histoDB ( struct LOG *log, struct DB *db, struct HISTODB *histo );
 extern gboolean Recuperer_histoDB ( struct LOG *log, struct DB *db );
 extern struct HISTODB *Recuperer_histoDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Modifier_histoDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_HISTO *histo );
 extern struct HISTODB *Rechercher_histoDB( struct LOG *log, struct DB *db, gint id );

 extern gboolean Ajouter_histo_hardDB ( struct LOG *log, struct DB *db, struct HISTO_HARDDB *histo );
 extern gboolean Creer_db_histo_hard ( struct LOG *log, struct DB *db );
 extern gboolean Rechercher_histo_hardDB ( struct LOG *log, struct DB *db,
                                           struct CMD_REQUETE_HISTO_HARD *critere );
 extern struct HISTO_HARDDB *Rechercher_histo_hardDB_suite( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
