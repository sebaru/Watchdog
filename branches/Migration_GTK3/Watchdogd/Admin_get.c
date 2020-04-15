/******************************************************************************************************************************/
/* Watchdogd/admin_get.c        Gestion des responses Admin GET au serveur watchdog                                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_get.c
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
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_get: Gere une commande 'admin get' depuis une response admin                                                         */
/* Entrée: le response et la ligne de commande                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_get ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[256];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'GET'" );

       response = Admin_write ( response, " | - new_e $tech_id:$acronyme   - Get digitalinput $tech_id:$acronyme" );
       response = Admin_write ( response, " | - new_b $tech_id:$acronyme   - Get bistable $tech_id:$acronyme" );
       response = Admin_write ( response, " | - new_m $tech_id:$acronyme   - Get monostable $tech_id:$acronyme" );
       response = Admin_write ( response, " | - t $tech_id:$acronyme       - Get Tempo $tech_id:$acronyme" );
       response = Admin_write ( response, " | - new_ea $tech_id:$acronyme  - Get Analog Input $tech_id:$acronyme" );
       response = Admin_write ( response, " | - new_msg $tech_id:$acronyme - Get Message $tech_id:$acronyme" );
       response = Admin_write ( response, " | - list_ea                   - List all dynamic Digital Inputs" );
       response = Admin_write ( response, " | - list $tech_id             - Get all bits of DLS $tech_id" );
       response = Admin_write ( response, " | - e $num                    - Get E[$num]" );
       response = Admin_write ( response, " | - ea $num                   - Get EA[$num]" );
       response = Admin_write ( response, " | - m $num                    - Get M[$num]" );
       response = Admin_write ( response, " | - b $num                    - Get B[$num]" );
       response = Admin_write ( response, " | - a $num                    - Get A[$num]" );
       response = Admin_write ( response, " | - i $num                    - Get I[$num]" );
       response = Admin_write ( response, " | - help                      - This help" );
     } else
    if ( ! strcmp ( commande, "t" ) )
     { gchar tech_id[80], acronyme[80];
       if (sscanf ( ligne, "%s %[^:]%s", commande, tech_id, acronyme ) == 3)               /* Découpage de la ligne de commande */
        { struct DLS_TEMPO *tempo = NULL;
          Dls_data_get_tempo ( tech_id, acronyme, (gpointer)&tempo );
          if (tempo)
           { g_snprintf( chaine, sizeof(chaine), " | - T: %s:%s -> delai_on=%d min_on=%d max_on=%d delai_off=%d", tech_id, acronyme,
			                   tempo->delai_on, tempo->min_on, tempo->max_on, tempo->delai_off );
             response = Admin_write ( response, chaine );
             g_snprintf( chaine, sizeof(chaine), " | - T: %s:%s = %d : status = %d, date_on=%d(%08.1fs) date_off=%d(%08.1fs)", tech_id, acronyme,
                      tempo->state, tempo->status, tempo->date_on,
                      (tempo->date_on > Partage->top ? (tempo->date_on - Partage->top)/10.0 : 0.0),
                      tempo->date_off,
                     (tempo->date_off > Partage->top ? (tempo->date_off - Partage->top)/10.0 : 0.0) );
             response = Admin_write ( response, chaine );
           } else
           { g_snprintf( chaine, sizeof(chaine), " | - T: %s_%s not found", tech_id, acronyme );
             response = Admin_write ( response, chaine );
	          }
        }
     } else
    if ( ! strcmp ( commande, "i" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_BIT_CONTROLE)
        { g_snprintf( chaine, sizeof(chaine), " | - I%03d = etat=%d, rouge=%d, vert=%d, bleu=%d, cligno=%d,\n"
                                              " |   changes=%d, last_change=%d, top=%d",
                      num, Partage->i[num].etat,
                      Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu,
                      Partage->i[num].cligno, Partage->i[num].changes, Partage->i[num].last_change,
                      Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - I -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "new_msg" ) )
     { gchar tech_id[80], acronyme[80];
       if (sscanf ( ligne, "%s %[^:]%s", commande, tech_id, acronyme ) == 3)               /* Découpage de la ligne de commande */
        { struct DLS_MESSAGES *msg = NULL;
          Dls_data_get_MSG ( tech_id, acronyme, (gpointer)&msg );
          if (msg)
           { g_snprintf( chaine, sizeof(chaine), " | - MSG %s:%s = %d, changes = %d, last_change = %d top=%d",
                      tech_id,acronyme, msg->etat, msg->changes, msg->last_change, Partage->top );
             response = Admin_write ( response, chaine );
           } else
           { g_snprintf( chaine, sizeof(chaine), " | - MSG: %s:%s not found", tech_id, acronyme );
             response = Admin_write ( response, chaine );
	          }
        }
     } else
    if ( ! strcmp ( commande, "m" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - M%03d = %d", num, M(num) );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "e" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - E%03d = %d",
                   num, E(num) );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "ea" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_ENTRE_ANA)
        { g_snprintf( chaine, sizeof(chaine),
                      " | - EA%03d = %8.2f %s, val_avant_ech=%8.2f, inrange=%d, type=%d, last_arch=%d (%ds ago), min=%8.2f, max=%8.2f",
                      num, EA_ech(num), Partage->ea[num].confDB.unite, Partage->ea[num].val_avant_ech, EA_inrange(num),
                      Partage->ea[num].confDB.type, Partage->ea[num].last_arch,
                      (Partage->top - Partage->ea[num].last_arch)/10,
                      Partage->ea[num].confDB.min, Partage->ea[num].confDB.max
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - EA -> num '%d' out of range (max=%d)", num, NBR_ENTRE_ANA ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar tech_id[80];
       GSList *liste;
       if (sscanf ( ligne, "%s %s", commande, tech_id ) == 2)                            /* Découpage de la ligne de commande */
        { liste = Partage->Dls_data_BOOL;
          while (liste)
           { struct DLS_BOOL *bool = liste->data;
             liste = g_slist_next(liste);
             if (strcmp(bool->tech_id, tech_id)) continue;
             g_snprintf( chaine, sizeof(chaine), " | - Bool '%s:%s' = %d", bool->tech_id, bool->acronyme, bool->etat );
             response = Admin_write ( response, chaine );
           }
          liste = Partage->Dls_data_BOOL;
          while (liste)
           { struct DLS_TEMPO *tempo = liste->data;
             liste = g_slist_next(liste);
             if (strcmp(tempo->tech_id, tech_id)) continue;
             g_snprintf( chaine, sizeof(chaine), " | - T: %s_%s -> delai_on=%d min_on=%d max_on=%d delai_off=%d",
                        tempo->tech_id, tempo->acronyme,
			                     tempo->delai_on, tempo->min_on, tempo->max_on, tempo->delai_off );
             response = Admin_write ( response, chaine );
             g_snprintf( chaine, sizeof(chaine), " | - T: %s_%s = %d : status = %d, date_on=%d(%08.1fs) date_off=%d(%08.1fs)",
                      tempo->tech_id, tempo->acronyme,
                      tempo->state, tempo->status, tempo->date_on,
                      (tempo->date_on > Partage->top ? (tempo->date_on - Partage->top)/10.0 : 0.0),
                      tempo->date_off,
                     (tempo->date_off > Partage->top ? (tempo->date_off - Partage->top)/10.0 : 0.0) );
             response = Admin_write ( response, chaine );
           }
          liste = Partage->Dls_data_AI;
          while (liste)
           { struct DLS_AI *ai = liste->data;
             liste = g_slist_next(liste);
             if (strcmp(ai->tech_id, tech_id)) continue;
             g_snprintf( chaine, sizeof(chaine),
                         " | - EA '%s:%s' = %8.2f %s, val_avant_ech=%8.2f, inrange=%d\n"
                         "                  type=%d, last_arch=%d (%ds ago), min=%8.2f, max=%8.2f",
                         ai->tech_id, ai->acronyme, ai->val_ech, ai->unite, ai->val_avant_ech, ai->inrange,
                         ai->type, ai->last_arch, (Partage->top - ai->last_arch)/10,
                         ai->min, ai->max
                       );
             response = Admin_write ( response, chaine );
           }
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - Usage: get list $tech_id" );
          response = Admin_write ( response, chaine );
	       }
     } else
    if ( ! strcmp ( commande, "b" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - B%03d = %d", num, B(num) );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "new_b" ) || ! strcmp ( commande, "new_m" ) )
     { gchar tech_id[80], acronyme[80];
       if (sscanf ( ligne, "%s %[^:]:%s", commande, tech_id, acronyme ) == 3)             /* Découpage de la ligne de commande */
        { g_snprintf( chaine, sizeof(chaine), " | - %s:%s = %d", tech_id, acronyme, Dls_data_get_bool ( tech_id, acronyme, NULL ) ); }
       else { g_snprintf( chaine, sizeof(chaine), " | - Wrong number of parameters" ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "new_e" ) )
     { gchar tech_id[80], acronyme[80];
       if (sscanf ( ligne, "%s %[^:]:%s", commande, tech_id, acronyme ) == 3)             /* Découpage de la ligne de commande */
        { g_snprintf( chaine, sizeof(chaine), " | - %s:%s = %d", tech_id, acronyme, Dls_data_get_DI ( tech_id, acronyme, NULL ) ); }
       else { g_snprintf( chaine, sizeof(chaine), " | - Wrong number of parameters" ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "a" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - A%03d = %d", num, A(num) );
       response = Admin_write ( response, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
