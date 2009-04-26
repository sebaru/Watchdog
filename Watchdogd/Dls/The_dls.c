/**********************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 avr 2009 19:54:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * The_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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

 #include "Cst_mnemoniques.h"
 #include "watchdogd.h"
 #include "Cst_dls.h"
 #include "Dls.h"

 static GList *Cde_exterieure=NULL;                      /* Numero des monostables mis à 1 via le serveur */
/**********************************************************************************************************/
/* Chrono: renvoi la difference de temps entre deux structures timeval                                    */
/* Entrée: le temps avant, et le temps apres l'action                                                     */
/* Sortie: un float                                                                                       */
/**********************************************************************************************************/
 static float Chrono ( struct timeval *avant, struct timeval *apres )
  { if (!(avant && apres)) return(0.0);
    else return( apres->tv_sec - avant->tv_sec + (apres->tv_usec - avant->tv_usec)/1000000.0 );
  }

/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int E( int num )
  { return( (num<NBR_ENTRE_TOR) && ((Partage->e[ num>>3 ]) & (1<<(num%8))) ); }

/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int EA_inrange( int num )
  { if (num<NBR_ENTRE_ANA) return(Partage->ea[ num ].inrange);
    else return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre ANA                                                                      */
/**********************************************************************************************************/
 int EA_int( int num )
  { if (num<NBR_ENTRE_ANA) return(Partage->ea[ num ].val);
    else return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 double EA_ech( int num )
  { if (num<NBR_ENTRE_ANA) return (Partage->ea[ num ].val_ech);
    else return(0.0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int EA_ech_inf( double val, int num )
  { if (num<NBR_ENTRE_ANA && EA_inrange(num) && (Partage->ea[ num ].val_ech >= val) ) return(0);
    else return(1);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int EA_ech_inf_egal( double val, int num )
  { if (num<NBR_ENTRE_ANA && EA_inrange(num) && (Partage->ea[ num ].val_ech > val) ) return(0);
    else return(1);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int EA_ech_sup( double val, int num )
  { if (num<NBR_ENTRE_ANA && EA_inrange(num) && (Partage->ea[ num ].val_ech <= val) ) return(0);
    else return(1);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int EA_ech_sup_egal( double val, int num )
  { if (num<NBR_ENTRE_ANA && EA_inrange(num) && (Partage->ea[ num ].val_ech < val) ) return(0);
    else return(1);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int A( int num )
  { return( (num<NBR_SORTIE_TOR) && ((Partage->a[ num>>3 ]) & (1<<(num%8))) ); }
  
/**********************************************************************************************************/
/* Renvoie la valeur d'un bistable                                                                        */
/**********************************************************************************************************/
 int B( int num )
  { return( (gint)((num<NBR_BIT_BISTABLE) && ((Partage->b[ num>>3 ]) & (1<<(num%8)))) ); }

/**********************************************************************************************************/
/* Renvoie la valeur d'un monostable                                                                      */
/**********************************************************************************************************/
 int M( int num )
  { return( (num<NBR_BIT_MONOSTABLE) && ((Partage->m[ num>>3 ]) & (1<<(num%8))) ); }

/**********************************************************************************************************/
/* Renvoie la valeur d'une tempo retard                                                                   */
/**********************************************************************************************************/
 int TR( int num )
  { return( (num<NBR_TEMPO) && Partage->Tempo_R[num].consigne &&
                              (Partage->Tempo_R[num].consigne<=Partage->top) );
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une tempo retard                                                                   */
/**********************************************************************************************************/
 char *TRdetail( int num )
  { static char chaine[90];
    snprintf( chaine, sizeof(chaine), "TR%d %d %d val %d", num, Partage->Tempo_R[num].consigne, Partage->top,
                                        Partage->Tempo_R[num].consigne<=Partage->top );
    return( chaine );
  }
/**********************************************************************************************************/
/* Renvoie la valeur complementée d'une tempo retard                                                      */
/**********************************************************************************************************/
 int TRbarre( int num )
  { return( (num<NBR_TEMPO) && Partage->Tempo_R[num].consigne &&
                              (Partage->top<Partage->Tempo_R[num].consigne) );
  }

/**********************************************************************************************************/
/* Met à jour l'entrée num                                                                                */
/**********************************************************************************************************/
 void SE( int num, int etat )
  { if (num>=NBR_ENTRE_TOR) return;

    if ( (E(num) && !etat) || (!E(num) && etat) )
     { Ajouter_arch( MNEMO_ENTREE, num, etat ); } 

    if (etat)
     { Partage->e[ num>>3 ] |=  (1<<(num%8)); }
    else
     { Partage->e[ num>>3 ] &= ~(1<<(num%8)); }
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num    val_int sur 12 bits !!                                           */
/**********************************************************************************************************/
 void SEA( int num, int val_int, int inrange )
  { if (num>=NBR_ENTRE_ANA) return;

    if (Partage->ea[ num ].val != val_int)
     { Partage->ea[ num ].val     = val_int;
       Ajouter_arch( MNEMO_ENTREE_ANA, num, val_int );
     }
    Partage->ea[ num ].val_ech =                                                    /* Valeur à l'echelle */
                     (((gdouble)val_int * (Partage->ea[num].max - Partage->ea[num].min)) / 4095)
                     + Partage->ea[num].min;
       
    Partage->ea[ num ].date    = time(NULL);   /* utilisé ?? */
    Partage->ea[ num ].inrange = inrange;
  }
/**********************************************************************************************************/
/* SB: Positionnement d'un bistable DLS                                                                   */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SB( int num, int etat )
  { gint numero, bit;
    if (num>=NBR_BIT_BISTABLE) return;

    /*if ( (!B(num) && !etat) || (B(num) && etat) ) return;*/
    numero = num>>3;
    bit = 1<<(num & 0x07);

    if (num>10) Partage->audit_bit_interne_per_sec++;                /* On ne compte pas les bits système */
    if (etat)
     { Partage->b[numero] |= bit; }
    else
     { Partage->b[numero] &= ~bit; }
  }
/**********************************************************************************************************/
/* SM: Positionnement d'un monostable DLS                                                                 */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SM( int num, int etat )
  { gint numero, bit;

    if (num>=NBR_BIT_MONOSTABLE) return;
    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat) Partage->m[numero] |= bit;
    else      Partage->m[numero] &= ~bit;
  }
/**********************************************************************************************************/
/* SI: Positionnement d'un motif TOR                                                                      */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SI( int num, int etat, int rouge, int vert, int bleu, int cligno )
  { if ( num>=NBR_BIT_CONTROLE ) return;
    if (Partage->i[num].etat   != etat || Partage->i[num].rouge!=rouge || 
        Partage->i[num].vert   != vert || Partage->i[num].bleu!=bleu ||
        Partage->i[num].cligno != cligno
       )
     { Partage->i[num].etat   = etat;
       Partage->i[num].rouge  = rouge;
       Partage->i[num].vert   = vert;
       Partage->i[num].bleu   = bleu;
       Partage->i[num].cligno = cligno;

       pthread_mutex_lock( &Partage->com_msrv.synchro );      /* Ajout dans la liste de msg a traiter */
       if ( ! g_list_find( Partage->com_msrv.liste_i, GINT_TO_POINTER(num) ) )
        { Partage->com_msrv.liste_i = g_list_append( Partage->com_msrv.liste_i,
                                                         GINT_TO_POINTER(num) );
        }
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* STR: Positionnement d'une Tempo retard DLS                                                             */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void STR( int num, int cons )
  { if (num>=NBR_TEMPO) return;                                              /* Si pas deja en decomptage */
    if (!cons) Partage->Tempo_R[num].consigne = 0;                                     /* Raz de la tempo */
    else                                               /* Initialisation tempo, si elle ne l'est pas deja */
    if (!Partage->Tempo_R[num].consigne) Partage->Tempo_R[num].consigne = Partage->top + cons;
  }

/**********************************************************************************************************/
/* SA: Positionnement d'un actionneur DLS                                                                 */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SA( int num, int etat )
  { gint numero, bit;
    if (num>=NBR_SORTIE_TOR) return;

    if ( (A(num) && !etat) || (!A(num) && etat) )
     { Ajouter_arch( MNEMO_SORTIE, num, etat ); 
       Partage->audit_bit_interne_per_sec++;
     }

    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat) { Partage->a[numero] |= bit;  }
         else { Partage->a[numero] &= ~bit; }
  }
/**********************************************************************************************************/
/* Met à jour le compteur horaire                                                                         */
/* Le compteur compte les MINUTES !!                                                                      */
/**********************************************************************************************************/
 void SCH( int num, int etat )
  { if (num>=NBR_COMPTEUR_H) return;
    if (etat)
     { if ( ! Partage->ch[ num ].actif )
        { Partage->ch[num].actif = TRUE;
          Partage->ch[num].old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - Partage->ch[num].old_top;
          if (delta > 600)
           { Partage->ch[num].cpthdb.valeur ++;
             Partage->ch[num].old_top = new_top;
           }
        }
     }
    else
     { Partage->ch[ num ].actif = FALSE; }
  }
/**********************************************************************************************************/
/* MSG: Envoi d'un message au serveur                                                                     */
/* Entrée: le numero du message, le type de clignotement                                                  */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void MSG( int num, int etat )
  { gint numero, bit, nbr;

    if ( num>=NBR_MESSAGE_ECRITS ) return;
    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat)
     { if (Partage->g[numero] & bit) return;
       Partage->g[numero] |= bit;

       pthread_mutex_lock( &Partage->com_msrv.synchro );      /* Ajout dans la liste de msg a traiter */
       nbr = g_list_length(Partage->com_msrv.liste_msg_on);
       if ( nbr < 200 && ! g_list_find( Partage->com_msrv.liste_msg_on, GINT_TO_POINTER(num) ) )
        { Partage->com_msrv.liste_msg_on = g_list_append( Partage->com_msrv.liste_msg_on,
                                                              GINT_TO_POINTER(num) );
        }
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
     }
    else
     { if (! (Partage->g[numero] & bit)) return;
       Partage->g[numero] &= ~bit;

       pthread_mutex_lock( &Partage->com_msrv.synchro );      /* Ajout dans la liste de msg a traiter */
       nbr = g_list_length(Partage->com_msrv.liste_msg_off);
       if ( nbr < 200 && ! g_list_find( Partage->com_msrv.liste_msg_off, GINT_TO_POINTER(num) ) )
        { Partage->com_msrv.liste_msg_off = g_list_append( Partage->com_msrv.liste_msg_off,
                                                               GINT_TO_POINTER(num) );
        }
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
     }
    Partage->audit_bit_interne_per_sec++;
  }
/**********************************************************************************************************/
/* Raz_cde_exterieure: Mise à zero des monostables de commande exterieure                                 */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Raz_cde_exterieure ( void )
  { GList *liste;
    liste = Cde_exterieure;
    while( liste )                                                               /* Reset des monostables */
     { SM( GPOINTER_TO_INT(liste->data), 0 );
       Info_n( Config.log, DEBUG_INFO, "DLS: Raz_cde_exterieure: mise a 0 du bit M",
               GPOINTER_TO_INT(liste->data) );
       liste = liste->next;
     }
    g_list_free( Cde_exterieure );
    Cde_exterieure = NULL;
  }
/*--------------------------------------------------------------------------------------------------------*/
/**********************************************************************************************************/
/* Main: Fonction principale du DLS                                                                       */
/**********************************************************************************************************/
 void Run_dls ( void )
  { struct itimerval timer;
    gint Update_heure=0;
    GList *plugins;

    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                   /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                            /* Active le timer */

    sleep(10);/* attente 10 secondes pour initialisation des bit internes et collection des infos modules */

    Info( Config.log, DEBUG_FORK, "DLS: demarrage" );                                        /* Log Start */
             
    Partage->com_dls.Plugins            = NULL;            /* Initialisation des variables du thread */
    Partage->com_dls.liste_m            = NULL;            /* Initialisation des variables du thread */
    Partage->com_dls.liste_plugin_off   = NULL;
    Partage->com_dls.liste_plugin_on    = NULL;
    Partage->com_dls.liste_plugin_reset = NULL;
    Prendre_heure();                                 /* On initialise les variables de gestion de l'heure */
    Charger_plugins();                                                      /* Chargement des modules dls */
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { struct timeval tv_avant, tv_apres;

       if (Partage->com_dls.sigusr1)
        { Partage->com_dls.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "DLS: Run_dls: SIGUSR1" );
          Lister_plugins();
        }

       if (Partage->top-Update_heure>=600)      /* Gestion des changements d'horaire (toutes les minutes) */
        { Prendre_heure ();                            /* Mise à jour des variables de gestion de l'heure */
          Update_heure=Partage->top;
        }

       if (Partage->com_dls.liste_m)                            /* A-t-on un monostable a allumer ?? */
        { gint num;
Info( Config.log, DEBUG_DLS, "dls: monostable a allumer debut" );
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_m->data );
          Partage->com_dls.liste_m = g_list_remove ( Partage->com_dls.liste_m, GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Info_n( Config.log, DEBUG_INFO, "DLS: Run_dls: mise a un du bit M", num );
          SM( num, 1 );
          Cde_exterieure = g_list_append( Cde_exterieure, GINT_TO_POINTER( num ) );
Info( Config.log, DEBUG_DLS, "dls: monostable a allumer fin" );
        }

       if (Partage->com_dls.liste_plugin_off)                      /* A-t-on un plugin a eteindre ?? */
        { gint num;
Info( Config.log, DEBUG_DLS, "dls: plugin off debut" );
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_plugin_off->data );
          Partage->com_dls.liste_plugin_off = g_list_remove ( Partage->com_dls.liste_plugin_off,
                                                                   GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Activer_plugins( num, FALSE );
Info( Config.log, DEBUG_DLS, "dls: plugin off fin" );
        }

       if (Partage->com_dls.liste_plugin_on)                        /* A-t-on un plugin a allumer ?? */
        { gint num;
Info( Config.log, DEBUG_DLS, "dls: plugin on debut" );
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_plugin_on->data );
          Partage->com_dls.liste_plugin_on = g_list_remove ( Partage->com_dls.liste_plugin_on,
                                                                  GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Activer_plugins( num, TRUE );
Info( Config.log, DEBUG_DLS, "dls: plugin off fin" );
        }

       if (Partage->com_dls.liste_plugin_reset)                     /* A-t-on un plugin a reseter ?? */
        { gint num;
Info( Config.log, DEBUG_DLS, "dls: plugin reset debut" );
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_plugin_reset->data );
          Partage->com_dls.liste_plugin_reset = g_list_remove ( Partage->com_dls.liste_plugin_reset,
                                                                     GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Reseter_un_plugin( num );
Info( Config.log, DEBUG_DLS, "dls: plugin reset fin" );
        }

       SB(0, !B(0));                                            /* Change d'etat tous les tours programme */
       SB(1, 0);                                                                   /* B1 est toujours à 0 */
       SB(2, 1);                                                                   /* B2 est toujours à 1 */
       SI(1, 1, 255, 0, 0, 0 );                                               /* Icone toujours à 1:rouge */

       plugins = Partage->com_dls.Plugins;
       while(plugins)                                            /* On execute tous les modules un par un */
        { struct PLUGIN_DLS *plugin_actuel;
          plugin_actuel = (struct PLUGIN_DLS *)plugins->data;

          if (plugin_actuel->on && plugin_actuel->go)
           { gettimeofday( &tv_avant, NULL );
             Partage->top_cdg_plugin_dls = 0;                               /* On reset le cdg plugin DLS */
             pthread_mutex_lock( &Partage->com_dls.synchro );
             plugin_actuel->go( plugin_actuel->starting );                          /* On appel le plugin */
             pthread_mutex_unlock( &Partage->com_dls.synchro );
             gettimeofday( &tv_apres, NULL );
             plugin_actuel->conso+=Chrono( &tv_avant, &tv_apres );
             plugin_actuel->starting = 0;
           }
          plugins = plugins->next;
        }
       SB(3, 1);                                  /* B3 est toujours à un apres le premier tour programme */

       Raz_cde_exterieure();                        /* Mise à zero des monostables de commande exterieure */
       usleep(1000);
       sched_yield();
     }
    Retirer_plugins();                                                    /* Dechargement des modules DLS */
    Info_n( Config.log, DEBUG_FORK, "Run_dls: DLS Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
