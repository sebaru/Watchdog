/******************************************************************************************************************************/
/* Watchdogd/Archive.c  Gestion des archivages des bit_internes Watchdog                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 09 mai 2012 12:44:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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

 #include <glib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <locale.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Ajouter_arch( gchar *tech_id, gchar *acronyme, gdouble valeur )
  { if (Config.instance_is_master == FALSE) return;                                  /* Les instances Slave n'archivent pas ! */

    Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Add Arch in list: '%s:%s'=%f", __func__, tech_id, acronyme, valeur );
    struct timeval tv;
    JsonNode *arch = Json_node_create ();
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    Json_node_add_string ( arch, "tech_id",   tech_id );
    Json_node_add_string ( arch, "acronyme",  acronyme );
    Json_node_add_double ( arch, "valeur",    valeur );
    Json_node_add_int    ( arch, "date_sec",  tv.tv_sec );
    Json_node_add_int    ( arch, "date_usec", tv.tv_usec );

    pthread_mutex_lock( &Partage->archive_liste_sync );                                /* Ajout dans la liste de arch a traiter */
    Partage->archive_liste = g_slist_append( Partage->archive_liste, arch );
    Partage->archive_liste_taille++;
    pthread_mutex_unlock( &Partage->archive_liste_sync );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
