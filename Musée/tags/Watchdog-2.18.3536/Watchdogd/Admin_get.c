/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_get.c        Gestion des responses Admin GET au serveur watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_get.c
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
 
 #include <glib.h>
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_get: Gere une commande 'admin get' depuis une response admin                                                         */
/* Entrée: le response et la ligne de commande                                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_get ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'SET'" );

       response = Admin_write ( response, "  e $num                - Get E[$num]" );
       response = Admin_write ( response, "  ea $num               - Get EA[$num]" );
       response = Admin_write ( response, "  m $num                - Get M[$num]" );
       response = Admin_write ( response, "  b $num                - Get B[$num]" );
       response = Admin_write ( response, "  a $num                - Get A[$num]" );
       response = Admin_write ( response, "  msg $num              - Get MSG[$num]" );
       response = Admin_write ( response, "  tr $num               - Get TR[$num]" );
       response = Admin_write ( response, "  i $num                - Get I[$num]" );
       response = Admin_write ( response, "  ci $num               - Get CI[$num]" );
       response = Admin_write ( response, "  ch $num               - Get CH[$num]" );
       response = Admin_write ( response, "  r $num                - Get Registre $num _R[$num]" );
       response = Admin_write ( response, "  help                  - This help" );
     } else
    if ( ! strcmp ( commande, "t" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_TEMPO)
        { g_snprintf( chaine, sizeof(chaine), " | - T%04d -> delai_on=%d min_on=%d max_on=%d delai_off=%d", num,
			                   Partage->Tempo_R[num].confDB.delai_on, Partage->Tempo_R[num].confDB.min_on,
			                   Partage->Tempo_R[num].confDB.max_on, Partage->Tempo_R[num].confDB.delai_off );
          response = Admin_write ( response, chaine );
          g_snprintf( chaine, sizeof(chaine), " | - T%04d  = %d : status = %d, date_on=%d(%08.1fs) date_off=%d(%08.1fs)", num,
                      Partage->Tempo_R[num].state, Partage->Tempo_R[num].status,
                      Partage->Tempo_R[num].date_on,
                     (Partage->Tempo_R[num].date_on > Partage->top ? (Partage->Tempo_R[num].date_on - Partage->top)/10.0 : 0.0),
                      Partage->Tempo_R[num].date_off,
                     (Partage->Tempo_R[num].date_off > Partage->top ? (Partage->Tempo_R[num].date_off - Partage->top)/10.0 : 0.0) );
          response = Admin_write ( response, chaine );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - T -> num '%d' out of range", num );
          response = Admin_write ( response, chaine );
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
    if ( ! strcmp ( commande, "msg" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_MESSAGE_ECRITS)
        { g_snprintf( chaine, sizeof(chaine), " | - MSG%03d = %d, persist = %d, changes = %d, last_change = %d top=%d",
                      num, Partage->g[num].etat, Partage->g[num].persist, Partage->g[num].changes,
                      Partage->g[num].last_change, Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - MSG -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
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
    if ( ! strcmp ( commande, "r" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_REGISTRE)
        { g_snprintf( chaine, sizeof(chaine),
                      " | - R%03d = %8.2f", num, R(num)
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - R -> num '%d' out of range (max=%d)", num, NBR_REGISTRE ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "b" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - B%03d = %d", num, B(num) );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "a" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " | - A%03d = %d", num, A(num) );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "ci" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_IMP)
        { g_snprintf( chaine, sizeof(chaine), " | - CI%03d = %8.2f, type=%d, actif=%d, unite=%s, multi=%8.2f, val1=%8.2f, val2=%8.2f",
                      num, Partage->ci[num].val_en_cours2 * Partage->ci[num].confDB.multi,
                      Partage->ci[num].confDB.type, Partage->ci[num].actif,
                      Partage->ci[num].confDB.unite, Partage->ci[num].confDB.multi,
                      Partage->ci[num].val_en_cours1, Partage->ci[num].val_en_cours2
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - CI -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "ch" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_H)
        { g_snprintf( chaine, sizeof(chaine), " | - CH%03d = %6d, actif=%d",
                      num, Partage->ch[num].confDB.valeur, Partage->ch[num].actif
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - CH -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
