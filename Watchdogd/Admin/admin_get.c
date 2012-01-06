/**********************************************************************************************************/
/* Watchdogd/Admin/admin_get.c        Gestion des connexions Admin GET au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_get: Gere une commande 'admin get' depuis une connexion admin                                    */
/* Entrée: le client et la ligne de commande                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_get ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'SET'\n" );

       Write_admin ( client->connexion, "  e num                 - Get E[num]\n" );
       Write_admin ( client->connexion, "  ea num                - Get EA[num]\n" );
       Write_admin ( client->connexion, "  m num                 - Get M[num]\n" );
       Write_admin ( client->connexion, "  b num                 - Get B[num]\n" );
       Write_admin ( client->connexion, "  a num                 - Get A[num]\n" );
       Write_admin ( client->connexion, "  msg num               - Get MSG[num]\n" );
       Write_admin ( client->connexion, "  tr num                - Get TR[num]\n" );
       Write_admin ( client->connexion, "  i num                 - Get I[num]\n" );
       Write_admin ( client->connexion, "  ci num                - Get CI[num]\n" );
       Write_admin ( client->connexion, "  ch num                - Get CH[num]\n" );
       Write_admin ( client->connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "tr" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_TEMPO)
        { g_snprintf( chaine, sizeof(chaine), " TR%03d = %d consigne %d, top=%d\n",
                      num, TR(num), Partage->Tempo_R[num].consigne, Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " TR -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "i" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_BIT_CONTROLE)
        { g_snprintf( chaine, sizeof(chaine), " I%03d = etat=%d, rouge=%d, vert=%d, bleu=%d, cligno=%d, "
                                              "changes=%d, last_change=%d, top=%d\n",
                      num, Partage->i[num].etat,
                      Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu,
                      Partage->i[num].cligno, Partage->i[num].changes, Partage->i[num].last_change,
                      Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " I -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "msg" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_MESSAGE_ECRITS)
        { g_snprintf( chaine, sizeof(chaine), " MSG%03d = %d, changes = %d, last_change = %d top=%d\n",
                      num, Partage->g[num].etat, Partage->g[num].changes,
                      Partage->g[num].last_change, Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " MSG -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "m" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " M%03d = %d\n", num, M(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "e" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " E%03d = %d\n", num, E(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ea" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_ENTRE_ANA)
        { g_snprintf( chaine, sizeof(chaine), " Partage->Top : %d\n", Partage->top );
          Write_admin ( client->connexion, chaine );
          g_snprintf( chaine, sizeof(chaine),
                      " EA%03d = %8.2f %s, val_int=%d, inrange=%d, type=%d, last_arch=%d, min=%8.2f, max=%8.2f\n",
                      num, EA_ech(num), Partage->ea[num].cmd_type_eana.unite, Partage->ea[num].val_int, EA_inrange(num),
                      Partage->ea[num].cmd_type_eana.type, Partage->ea[num].last_arch, 
                      Partage->ea[num].cmd_type_eana.min, Partage->ea[num].cmd_type_eana.max 
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " EA -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "b" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " B%03d = %d\n", num, B(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "a" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       g_snprintf( chaine, sizeof(chaine), " A%03d = %d\n", num, A(num) );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ci" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_IMP)
        { g_snprintf( chaine, sizeof(chaine), " CI%03d = %8.2f, type=%d, actif=%d, unite=%s, multi=%8.2f, val1=%8.2f, val2=%8.2f\n",
                      num, Partage->ci[num].cpt_impdb.valeur, Partage->ci[num].cpt_impdb.type, Partage->ci[num].actif,
                      Partage->ci[num].cpt_impdb.unite, Partage->ci[num].cpt_impdb.multi,
                      Partage->ci[num].val_en_cours1, Partage->ci[num].val_en_cours2
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " CI -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ch" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       if (num<NBR_COMPTEUR_H)
        { g_snprintf( chaine, sizeof(chaine), " CH%03d = %6d, actif=%d\n",
                      num, Partage->ch[num].cpthdb.valeur, Partage->ch[num].actif
                    );
        } else
        { g_snprintf( chaine, sizeof(chaine), " CH -> num '%d' out of range\n", num ); }
       Write_admin ( client->connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
