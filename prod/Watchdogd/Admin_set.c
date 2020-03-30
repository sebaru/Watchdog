/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_set.c        Gestion des connexions Admin SET au serveur watchdog                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        jeu. 05 janv. 2012 23:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_set.c
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
/* Admin_set: Gere une commande 'admin set' depuis une connexion admin                                                        */
/* Entrée: La connexion connexione et la ligne de commande, et le buffer de sortie                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_set ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'SET'" );
       response = Admin_write ( response, " | - new_b $tech_id:$acro $val - Set bistable $tech_id:$acro to $val" );
       response = Admin_write ( response, " | - new_m $tech_id:$acro    - Set monostablea $tech_id:$acro to 1" );
       response = Admin_write ( response, " | - e num val               - Set E[num]   = val" );
       response = Admin_write ( response, " | - m num val               - Set M[num]   = val" );
       response = Admin_write ( response, " | - b num val               - Set B[num]   = val" );
       response = Admin_write ( response, " | - a num val               - Set A[num]   = val" );
       response = Admin_write ( response, " | - i num E R V B C         - Set I[num]   = Etat Rouge Vert Bleu Cligno" );
       response = Admin_write ( response, " | - help                    - This help" );
     } else
    if ( ! strcmp ( commande, "i" ) )
     { int num, etat, rouge, vert, bleu, cligno;                                         /* Découpage de la ligne de commande */
       sscanf ( ligne, "%s %d %d %d %d %d %d", commande, &num, &etat, &rouge, &vert, &bleu, &cligno );
       if (num<NBR_BIT_CONTROLE)
        { SI(num, etat, rouge, vert, bleu, cligno);
          sleep(1);
          g_snprintf( chaine, sizeof(chaine), " | - I%03d = etat=%d, rouge=%d, vert=%d, bleu=%d, cligno=%d, "
                                              "changes=%d, last_change=%d, top=%d",
                      num, Partage->i[num].etat,
                      Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu,
                      Partage->i[num].cligno, Partage->i[num].changes, Partage->i[num].last_change,
                      Partage->top );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - I -> num '%d' out of range", num ); }
        response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "e" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );                               /* Découpage de la ligne de commande */
       if (num<NBR_ENTRE_TOR)
        { if (val) val = 1;
          SE( num, val );                                                                                 /* Pas de furtivité */
          g_snprintf( chaine, sizeof(chaine), " | - E%03d = %d", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - E -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "m" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );                               /* Découpage de la ligne de commande */
       if (num<NBR_BIT_MONOSTABLE)
        { if (val) val = 1;
          if (val) Envoyer_commande_dls( num );
              else SM ( num, 0 );
          g_snprintf( chaine, sizeof(chaine), " | - M%03d = %d", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - M -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "b" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );                               /* Découpage de la ligne de commande */
       if (num<NBR_BIT_BISTABLE)
        { if (val) val = 1;
          SB ( num, val );
          g_snprintf( chaine, sizeof(chaine), " | - B%03d = %d", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - B -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "new_b" ) )
     { gchar tech_id[80], acronyme[80];
       int val;
       if (sscanf ( ligne, "%s %[^:]:%s %d", commande, tech_id, acronyme, &val ) == 4)   /* Découpage de la ligne de commande */
        { Dls_data_set_bool ( tech_id, acronyme, NULL, val );
          g_snprintf( chaine, sizeof(chaine), " | - %s:%s set to %d", tech_id, acronyme, val );
        }
       else { g_snprintf( chaine, sizeof(chaine), " | - Wrong number of parameters" ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "new_m" ) )
     { gchar tech_id[80], acronyme[80];
       if (sscanf ( ligne, "%s %[^:]:%s", commande, tech_id, acronyme ) == 3)            /* Découpage de la ligne de commande */
        { Envoyer_commande_dls_data ( tech_id, acronyme );
          g_snprintf( chaine, sizeof(chaine), " | - %s:%s set to 1", tech_id, acronyme );
        }
       else { g_snprintf( chaine, sizeof(chaine), " | - Wrong number of parameters" ); }
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "a" ) )
     { int num, val;
       sscanf ( ligne, "%s %d %d", commande, &num, &val );                               /* Découpage de la ligne de commande */
       if (num<NBR_SORTIE_TOR)
        { if (val) val = 1;
          SA ( num, val );
          g_snprintf( chaine, sizeof(chaine), " | - A%03d = %d", num, val );
        } else
        { g_snprintf( chaine, sizeof(chaine), " | - A -> num '%d' out of range", num ); }
       response = Admin_write ( response, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
