/**********************************************************************************************************/
/* Include/Erreur.h      Déclaration des constantes et prototypes de gestion de log                       */
/* Projet WatchDog version 1.7       Gestion d'habitat                      lun 21 avr 2003 22:06:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Erreur.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

#ifndef _ERREUR_H_
 #define _ERREUR_H_

 #include <glib.h>

 #define TAILLE_ENTETE_LOG   40
 struct LOG { char entete [TAILLE_ENTETE_LOG+1];
              guint debug_level;
            };

 #define  fonction_entre(); { printf("Entre dans la fonction %s\n", __FUNCTION__ ); }
 #define  fonction_sort();  { printf("Sort de la fonction %s\n", __FUNCTION__ ); }
 
 #define  DEBUG_SIGNAUX             (1<<0 ) /* SIGTERM */
 #define  DEBUG_DB                  (1<<1 ) /* Acces DB */
 #define  DEBUG_USER                (1<<2 ) /* Connexion user */
 #define  DEBUG_CONFIG              (1<<3 ) /* Chargement du fichier de conf */
 #define  DEBUG_CRYPTO              (1<<4 ) /* Crytage des clefs */
 #define  DEBUG_INFO                (1<<5 ) /* Pour info */
 #define  DEBUG_MEM                 (1<<6 ) /* Allocation memoire */
 #define  DEBUG_CDG                 (1<<7 ) /* Chiens de garde */
 #define  DEBUG_NETWORK             (1<<8 ) /* Debug reseau */
 #define  DEBUG_FORK                (1<<9 ) /* Debug fork process */
 #define  DEBUG_CONNEXION           (1<<10) /* Debug connexions clientes */
 #define  DEBUG_DLS                 (1<<11) /* Debug modules DLS */
 #define  DEBUG_MODBUS              (1<<12) /* Debug modules MODBUS */
 #define  DEBUG_ADMIN               (1<<13) /* Debug connexion ADMIN */
 #define  DEBUG_RS485               (1<<14) /* Debug modules RS485 */
 #define  DEBUG_ONDULEUR            (1<<15) /* Debug modules ONDULEUR */

 extern struct LOG *Info_init( gchar *entete, guint debug );
 extern void Info( struct LOG *log, guint niveau, gchar *texte );
 extern void Info_n( struct LOG *log, guint niveau, gchar *texte, gint valeur );
 extern void Info_c( struct LOG *log, guint niveau, gchar *texte, gchar *texte2 );
 extern void Info_change_debug( struct LOG *log, guint debug );

#endif
/*--------------------------------------------------------------------------------------------------------*/
