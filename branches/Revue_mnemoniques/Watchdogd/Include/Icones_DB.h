/**********************************************************************************************************/
/* Watchdogd/Include/Icones_DB.h        Déclaration structure internes des icones watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:33:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Icones_DB.h
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
 
#ifndef _ICONES_H_
 #define _ICONES_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_CLASSE     "class"
 #define NOM_TABLE_ICONE      "icons"

 struct CLASSEDB
  { gchar libelle[NBR_CARAC_CLASSE_ICONE_UTF8+1];
    guint id;
  };

 struct ICONEDB
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[NBR_CARAC_LIBELLE_ICONE_UTF8+1];
    guint id_classe;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern struct ICONEDB *Rechercher_iconeDB ( guint id );
 extern gboolean Recuperer_iconeDB ( struct DB **db, guint classe );
 extern struct ICONEDB *Recuperer_iconeDB_suite( struct DB **db );
 extern gint Ajouter_iconeDB ( struct CMD_TYPE_ICONE *icone );
 extern gboolean Retirer_iconeDB ( struct CMD_TYPE_ICONE *icone );
 extern gboolean Modifier_iconeDB( struct CMD_TYPE_ICONE *icone );

 extern struct CLASSEDB *Rechercher_classeDB ( guint id );
 extern gboolean Recuperer_classeDB ( struct DB **db );
 extern struct CLASSEDB *Recuperer_classeDB_suite( struct DB **db );
 extern gint Ajouter_classeDB ( struct CMD_TYPE_CLASSE *classe );
 extern gboolean Retirer_classeDB ( struct CMD_TYPE_CLASSE *classe );
 extern gboolean Modifier_classeDB( struct CMD_TYPE_CLASSE *classe );
 extern gint Icone_get_data_version ( void );
 extern void Icone_set_data_version ( void );
#endif
/*--------------------------------------------------------------------------------------------------------*/
