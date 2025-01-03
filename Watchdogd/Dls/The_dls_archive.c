/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_archive.c  Gestion des archives                                                                      */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                                27.11.2022 18:09:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_archive.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
 #include <fcntl.h>
 #include <string.h>
 #include <signal.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <semaphore.h>
 #include <locale.h>
 #include <math.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_run_archivage: Gere l'archivage des bits internes le necessitant                                                       */
/* Entrée : le plugin a traiter                                                                                               */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_run_archivage ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { if (!plugin) return;
    if (!plugin->enable) return;                                                        /* On archive pas les plugins disable */

    GSList *liste = plugin->Dls_data_AI;
    while ( liste )
     { struct DLS_AI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, (bit->in_range ? bit->valeur : 0.0) );            /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_DI;
    while ( liste )
     { struct DLS_DI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->etat*1.0 );                                  /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_DO;
    while ( liste )
     { struct DLS_DO *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->etat*1.0 );                                  /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CH;
    while ( liste )
     { struct DLS_CH *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_REGISTRE;
    while ( liste )
     { struct DLS_REGISTRE *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Partage->top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { MQTT_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Partage->top;
        }
       liste = g_slist_next ( liste );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
