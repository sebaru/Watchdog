/**********************************************************************************************************/
/* Watchdogd/Include/Mnemonique.h        Déclaration structure internes des mnemoniques watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 21 jan 2004 18:45:59 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Mnemonique_DB.h
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
 
#ifndef _MNEMONIQUE_H_
 #define _MNEMONIQUE_H_

 #include <glib.h>
 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_MNEMO    "mnemos"

/*************************************** Définitions des prototypes ***************************************/
 extern struct CMD_TYPE_MNEMONIQUE *Rechercher_mnemoDB ( guint id );
 extern gboolean Recuperer_mnemoDB ( struct DB **db );
 extern gboolean Recuperer_mnemoDB_for_courbe ( struct DB **db );
 extern gboolean Recuperer_mnemoDB_by_command_text ( struct DB **db,
                                                     gchar *commande_pure, gboolean exact );
 extern struct CMD_TYPE_MNEMONIQUE *Recuperer_mnemoDB_suite( struct DB **db );
 extern gint Ajouter_mnemoDB ( struct CMD_TYPE_MNEMONIQUE *mnemo );
 extern gboolean Retirer_mnemoDB ( struct CMD_TYPE_MNEMONIQUE *mnemo );
 extern gboolean Modifier_mnemoDB( struct CMD_TYPE_MNEMONIQUE *mnemo );
 extern struct CMD_TYPE_MNEMONIQUE *Rechercher_mnemoDB_type_num ( struct CMD_TYPE_NUM_MNEMONIQUE *critere );

#endif
/*--------------------------------------------------------------------------------------------------------*/
