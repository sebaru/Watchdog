/**********************************************************************************************************/
/* Watchdogd/shm.c        Gestion de la m�moire partag�e                                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                      dim 05 avr 2009 12:32:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * shm.c
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
 #include <sys/ipc.h>
 #include <sys/shm.h>

 #include "watchdogd.h"
/**********************************************************************************************************/
/* Shm_init: initialisation de la m�moire partag�e                                                        */
/* Entr�e: rien                                                                                           */
/* Sortie: un pointeur sur la zone m�moire partag�e                                                       */
/**********************************************************************************************************/
 struct PARTAGE *Shm_init ( void )
  { struct PARTAGE *partage;
    gint taille;

    taille = sizeof( struct PARTAGE );                                      /* Le jeton, les comms, les I */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Shm_init: size required = %d", taille );

    partage = g_try_malloc0( taille );
    return(partage);
  }
/**********************************************************************************************************/
/* Shm_stop: Stoppe l'utilisation de la m�moire partag�e                                                  */
/* Entr�e: une structure de m�moire partag�e                                                              */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Shm_stop ( struct PARTAGE *partage )
  { Info_new( Config.log, Config.log_msrv, LOG_INFO, "Shm_stop: freeing memory" );
    g_free(Partage);
    return(TRUE);
  }
/**********************************************************************************************************/
/* w_malloc0: Permet de trapper les demandes de reservation m�moire                                       */
/* Entr�e: une taille et une justification                                                                */
/* Sortie: un pointeur                                                                                    */
/**********************************************************************************************************/
 void *w_malloc0( gint size, gchar *justification )
  { void *ptr;
    Info_new( Config.log, TRUE, LOG_DEBUG,
              "w_malloc0: %d bytes requested for %s", size, justification );
    ptr = g_try_malloc0 ( size );
    Info_new( Config.log, TRUE, LOG_DEBUG,
              "w_malloc0: %d bytes given for %s, ptr = %p", size, justification, ptr );
    return(ptr);
  }
/**********************************************************************************************************/
/* w_malloc0: Permet de trapper les demandes de reservation m�moire                                       */
/* Entr�e: une taille et une justification                                                                */
/* Sortie: un pointeur                                                                                    */
/**********************************************************************************************************/
 void w_free( void *ptr, gchar *justification )
  { Info_new( Config.log, TRUE, LOG_DEBUG,
              "w_free: Released request for %s, ptr = %p", justification, ptr );
    g_free ( ptr );
    Info_new( Config.log, TRUE, LOG_DEBUG,
              "w_free: Released done for %s, ptr = %p", justification, ptr );
  }
/*--------------------------------------------------------------------------------------------------------*/
