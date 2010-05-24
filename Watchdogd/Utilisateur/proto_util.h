/**********************************************************************************************************/
/* Watchdog/Utilisateur/proto_util.h    Header locaux                                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 21 avr 2003 13:09:57 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * proto_util.h
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
 
 #include "Utilisateur_DB.h"

 extern gchar *Groupes_vers_string ( guint *source );
 extern gint Get_login_failed( struct LOG *log, struct DB *db, guint id );
 extern gboolean Ajouter_one_login_failed( struct LOG *log, struct DB *db, guint id, gint max_login_failed );
 extern gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable );
 extern guint *String_vers_groupes ( gchar *source );
 extern gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable );

/*--------------------------------------------------------------------------------------------------------*/
