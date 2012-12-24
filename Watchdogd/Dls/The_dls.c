/**********************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                  lmar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * The_dls.c
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
 #include <fcntl.h>
 #include <string.h>
 #include <signal.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <semaphore.h>

 #include "watchdogd.h"

 static GList *Cde_exterieure=NULL;                      /* Numero des monostables mis à 1 via le serveur */
 struct BIT_A_CHANGER
  { gint num;
    gint actif;
  };
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
  { if ( (num>=0) && (num<NBR_ENTRE_TOR) ) return ( ((Partage->e[ num>>3 ]) & (1<<(num%8)) ? 1 : 0) );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "E : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* EA_inrange : Renvoie 1 si l'EA en paramètre est dans le range de mesure                                */
/**********************************************************************************************************/
 int EA_inrange( int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) return( Partage->ea[ num ].inrange);
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_range : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech : Renvoie la valeur de l'EA interprétée (mis à l'échelle)                                       */
/**********************************************************************************************************/
 double EA_ech( int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) return (Partage->ea[ num ].val_ech);
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_ech : num out of range", num );
    return(0.0);
  }
/**********************************************************************************************************/
/* EA_ech_inf : Teste si la valeur de l'EA est inf à une mesure                                           */
/**********************************************************************************************************/
 int EA_ech_inf( double val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) < val); }
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_ech_inf : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_inf_egal : Teste si la valeur de l'EA est inf ou egale à une mesure                             */
/**********************************************************************************************************/
 int EA_ech_inf_egal( double val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) <= val); }
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_ech_inf_egal : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_sup : Teste si la valeur de l'EA est sup à une mesure                                           */
/**********************************************************************************************************/
 int EA_ech_sup( double val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) > val); }
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_ech_sup : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_sup_egal : Teste si la valeur de l'EA est sup ou egale à une mesure                             */
/**********************************************************************************************************/
 int EA_ech_sup_egal( double val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) >= val); }
    else Info_new( Config.log, Config.log_all, LOG_INFO, "EA_ech_sup_egal : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 float CI( int num )
  { if (num<NBR_COMPTEUR_IMP) return (Partage->ci[ num ].cpt_impdb.valeur);
    else Info_new( Config.log, Config.log_all, LOG_INFO, "CI : num out of range", num );
    return(0.0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int A( int num )
  { if ( num>=0 && num<NBR_SORTIE_TOR ) return ( Partage->a[ num ].etat );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "A : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'un bistable                                                                        */
/**********************************************************************************************************/
 int B( int num )
  { if (num>=0 && num<NBR_BIT_BISTABLE) return( ((Partage->b[ num>>3 ]) & (1<<(num%8)) ? 1 : 0 ) );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "B : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'un monostable                                                                      */
/**********************************************************************************************************/
 int M( int num )
  { if (num>=0 && num<NBR_BIT_MONOSTABLE) return( ((Partage->m[ num>>3 ]) & (1<<(num%8)) ? 1 : 0 ) );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "M : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une tempo retard                                                                   */
/**********************************************************************************************************/
 int TR( int num )
  { if (num>=0 && num<NBR_TEMPO) return ( Partage->Tempo_R[num].consigne &&
                                         (Partage->Tempo_R[num].consigne<=Partage->top) );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "TR : num out of range", num );
    return(0);
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
/* Renvoie la valeur TRUE si la temporisation est en cours de comptage, FALSE sinon                       */
/**********************************************************************************************************/
 int TRCount( int num )
  { if (num>=0 && num<NBR_TEMPO) return ( Partage->Tempo_R[num].consigne &&
                                         (Partage->top<Partage->Tempo_R[num].consigne) );
    else Info_new( Config.log, Config.log_all, LOG_INFO, "TRCount : num out of range", num );
    return(0);
  }
/**********************************************************************************************************/
/* Met à jour l'entrée num                                                                                */
/**********************************************************************************************************/
 void SE( int num, int etat )
  { if (num<0 || num>=NBR_ENTRE_TOR)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SE : num out of range", num );
       return;
     }

    if ( (E(num) && !etat) || (!E(num) && etat) )
     { Ajouter_arch( MNEMO_ENTREE, num, 1.0*etat ); } 

    if (etat)
     { Partage->e[ num>>3 ] |=  (1<<(num%8)); }
    else
     { Partage->e[ num>>3 ] &= ~(1<<(num%8)); }
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num    val_avant_ech sur 12 bits !!                                           */
/**********************************************************************************************************/
 void SEA_range( int num, int range )
  { if (num<0 || num>=NBR_ENTRE_ANA)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SEA_range : num out of range", num );
       return;
     }
    Partage->ea[num].inrange = range;
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num    val_avant_ech sur 12 bits !!                                           */
/**********************************************************************************************************/
 void SEA( int num, double val_avant_ech )
  { if (num<0 || num>=NBR_ENTRE_ANA)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SEA : num out of range", num );
       return;
     }

    if (Partage->ea[ num ].val_avant_ech != val_avant_ech)
     { Partage->ea[ num ].val_avant_ech = val_avant_ech;        /* Archive au mieux toutes les 5 secondes */
       if ( Partage->ea[ num ].last_arch + ARCHIVE_EA_TEMPS_SI_VARIABLE < Partage->top )
        { Ajouter_arch( MNEMO_ENTREE_ANA, num, val_avant_ech );
          Partage->ea[ num ].last_arch = Partage->top;   
        }
       switch ( Partage->ea[num].cmd_type_eana.type )
        { case ENTREEANA_NON_INTERP:
               Partage->ea[ num ].val_ech = val_avant_ech;                     /* Pas d'interprétation !! */
               Partage->ea[ num ].inrange = 1;
               break;
          case ENTREEANA_4_20_MA_10BITS:
               if (val_avant_ech < 100)                            /* 204) Modification du range pour 4mA */
                { Partage->ea[ num ].val_ech = 0.0;                                 /* Valeur à l'echelle */ 
                  Partage->ea[ num ].inrange = 0;
                }
               else
                { if (val_avant_ech < 204) val_avant_ech = 204;
                  Partage->ea[ num ].val_ech = (gdouble)
                  ((val_avant_ech-204)*(Partage->ea[num].cmd_type_eana.max - Partage->ea[num].cmd_type_eana.min))/820.0
                  + Partage->ea[num].cmd_type_eana.min;                             /* Valeur à l'echelle */ 

                  Partage->ea[ num ].inrange = 1;
                }
               break;
          case ENTREEANA_4_20_MA_12BITS:
               if (val_avant_ech < 400)
                { Partage->ea[ num ].val_ech = 0.0;                                 /* Valeur à l'echelle */ 
                  Partage->ea[ num ].inrange = 0;
                }
               else
                { if (val_avant_ech < 816) val_avant_ech = 816;
                  Partage->ea[ num ].val_ech = (gdouble)
                  ((val_avant_ech-816)*(Partage->ea[num].cmd_type_eana.max - Partage->ea[num].cmd_type_eana.min))/3280.0
                     + Partage->ea[num].cmd_type_eana.min;                          /* Valeur à l'echelle */ 
                  Partage->ea[ num ].inrange = 1;
                }
               break;
          case ENTREEANA_WAGO_750455:
               Partage->ea[ num ].val_ech = (gdouble)
                  (val_avant_ech*(Partage->ea[num].cmd_type_eana.max - Partage->ea[num].cmd_type_eana.min))/4095.0
                     + Partage->ea[num].cmd_type_eana.min;                          /* Valeur à l'echelle */ 
               break;
          case ENTREEANA_WAGO_750461:
               Partage->ea[ num ].val_ech = (gdouble)(val_avant_ech/10.0);                /* Valeur à l'echelle */ 
               break;
          default:
               Partage->ea[num].val_ech = 0.0;
        }
     }
    else if ( Partage->ea[ num ].last_arch + ARCHIVE_EA_TEMPS_SI_CONSTANT < Partage->top )
     { Ajouter_arch( MNEMO_ENTREE_ANA, num, val_avant_ech );               /* Archive au pire toutes les 10 min */
       Partage->ea[ num ].last_arch = Partage->top;   
     }
                                                                     /* Gestion historique interne Valana */
  }
/**********************************************************************************************************/
/* SB: Positionnement d'un bistable DLS                                                                   */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SB( int num, int etat )
  { gint numero, bit;
    if (num<0 || num>=NBR_BIT_BISTABLE)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SB : num out of range", num );
       return;
     }
    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat)                                                                       /* Mise a jour du bit */
     { Partage->b[numero] |= bit; 
       Partage->audit_bit_interne_per_sec++;
     }
    else
     { Partage->b[numero] &= ~bit;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* SM: Positionnement d'un monostable DLS                                                                 */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SM( int num, int etat )
  { gint numero, bit;
    if (num<0 || num>=NBR_BIT_MONOSTABLE)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SM : num out of range", num );
       return;
     }
    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat)                                                                       /* Mise a jour du bit */
     { Partage->m[numero] |= bit; 
       Partage->audit_bit_interne_per_sec++;
     }
    else
     { Partage->m[numero] &= ~bit;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* SI: Positionnement d'un motif TOR                                                                      */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SI( int num, int etat, int rouge, int vert, int bleu, int cligno )
  { gint nbr;
    if ( num<0 || num>=NBR_BIT_CONTROLE )
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SI : num out of range", num );
       return;
     }

    if (Partage->i[num].etat   != etat || Partage->i[num].rouge!=rouge || 
        Partage->i[num].vert   != vert || Partage->i[num].bleu!=bleu ||
        Partage->i[num].cligno != cligno
       )
     { Partage->i[num].etat   = etat;
       Partage->i[num].rouge  = rouge;
       Partage->i[num].vert   = vert;
       Partage->i[num].bleu   = bleu;
       Partage->i[num].cligno = cligno;

       if ( Partage->i[num].last_change + 10 <= Partage->top )   /* Si pas de change depuis plus de 1 sec */
        { Partage->i[num].changes = 0; }

       if ( Partage->i[num].changes <= 5 )                           /* Si moins de 5 changes par seconde */
        {
          if ( Partage->i[num].changes == 5 )                   /* Est-ce le dernier change avant blocage */
           { Partage->i[num].etat   = 0;                     /* Si oui, on passe le visuel en kaki cligno */
             Partage->i[num].rouge  = 0;
             Partage->i[num].vert   = 100;
             Partage->i[num].bleu   = 0;
             Partage->i[num].cligno = 1;
           }

          pthread_mutex_lock( &Partage->com_msrv.synchro );         /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_i = g_slist_append( Partage->com_msrv.liste_i,
                                                      GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Partage->i[num].changes++;                             /* Un change de plus ! */
        }
       Partage->i[num].last_change = Partage->top;                                    /* Date de la photo ! */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* STR: Positionnement d'une Tempo retard DLS                                                             */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void STR( int num, int cons )
  { if (num<0 || num>=NBR_TEMPO)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "STR: num out of range", num );
       return;
     }
                                                                             /* Si pas deja en decomptage */
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
  { if (num<0 || num>=NBR_SORTIE_TOR)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_all, LOG_INFO, "SA : num out of range", num ); }
       return;
     }

    if ( Partage->a[num].etat != etat )
     { Partage->a[num].etat = etat;

       if ( Partage->a[num].last_change + 10 <= Partage->top )   /* Si pas de change depuis plus de 1 sec */
        { Partage->a[num].changes = 0; }

       if ( Partage->a[num].changes <= 5 )/* Arbitraire : si plus de 5 changes dans la seconde, on bloque */
        { Ajouter_arch( MNEMO_SORTIE, num, 1.0*etat );
          pthread_mutex_lock( &Partage->com_msrv.synchro );       /* Ajout dans la liste de msg a traiter */
          Partage->com_msrv.liste_a = g_slist_append( Partage->com_msrv.liste_a,
                                                      GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Partage->a[num].changes++;                                              /* Un change de plus !! */
        } else if ( ! (Partage->top % 600 ))             /* Si persistence on prévient toutes les minutes */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "SA: last_change trop tôt !", num ); }
       Partage->a[num].last_change = Partage->top;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* Met à jour le compteur horaire                                                                         */
/* Le compteur compte les MINUTES !!                                                                      */
/**********************************************************************************************************/
 void SCH( int num, int etat )
  { if (num<0 || num>=NBR_COMPTEUR_H)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "SCH : num out of range", num );
       return;
     }
    if (etat)
     { if ( ! Partage->ch[ num ].actif )
        { Partage->ch[num].actif = TRUE;
          Partage->ch[num].old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - Partage->ch[num].old_top;
          if (delta > 600)                                           /* On compte +1 toutes les minutes ! */
           { Partage->ch[num].cpthdb.valeur ++;
             Partage->ch[num].old_top = new_top;
           }
        }
     }
    else
     { Partage->ch[ num ].actif = FALSE; }
  }
/**********************************************************************************************************/
/* Met à jour le compteur impulsion                                                                       */
/* Le compteur compte les impulsions !!                                                                   */
/**********************************************************************************************************/
 void SCI( int num, int etat, int reset, int ratio )
  { if (num<0 || num>=NBR_COMPTEUR_IMP)
     { Info_new( Config.log, Config.log_all, LOG_INFO, "CI : num out of range", num );
       return;
     }
    if (etat)
     { if (reset)                                                   /* Le compteur doit-il etre resetté ? */
        { Partage->ci[num].val_en_cours1 = 0.0;                /* Valeur transitoire pour gérer les ratio */
          Partage->ci[num].val_en_cours2 = 0.0;                /* Valeur transitoire pour gérer les ratio */
        }

       if ( ! Partage->ci[ num ].actif )                                              /* Passage en actif */
        { Partage->ci[num].actif = TRUE;
          Partage->ci[num].val_en_cours1++;
          if (Partage->ci[num].val_en_cours1>=ratio)
           { Partage->ci[num].val_en_cours2++;
             Partage->ci[num].val_en_cours1=0.0;                          /* RAZ de la valeur de calcul 1 */
           }
        }
     }
    else
     { if (!reset) Partage->ci[ num ].actif = FALSE; }

    switch (Partage->ci[ num ].cpt_impdb.type)                        /* Calcul de la valeur réelle du CI */
     { case CI_TOTALISATEUR : if ( Partage->ci[num].last_update + 1 < Partage->top )
                               { Partage->ci[num].cpt_impdb.valeur = Partage->ci[num].val_en_cours2;
                                 Partage->ci[num].last_update = Partage->top;
                               }
                              break;
       case CI_MOYENNEUR_SEC: if ( Partage->ci[num].last_update + 10 < Partage->top )
                               { Partage->ci[num].cpt_impdb.valeur = (Partage->ci[num].cpt_impdb.valeur +
                                                                      Partage->ci[num].val_en_cours2)/2.0;
                                 Partage->ci[num].val_en_cours2 = 0.0;
                                 Partage->ci[num].last_update = Partage->top;
                               }
                              break;
       case CI_MOYENNEUR_MIN: if ( Partage->ci[num].last_update + 600 < Partage->top )
                               { Partage->ci[num].cpt_impdb.valeur = (Partage->ci[num].cpt_impdb.valeur +
                                                                      Partage->ci[num].val_en_cours2)/2.0;
                                 Partage->ci[num].val_en_cours2 = 0.0;
                                 Partage->ci[num].last_update = Partage->top;
                               }
                              break;
     }
  }
/**********************************************************************************************************/
/* MSG: Positionnement des message DLS                                                                    */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void MSG( int num, int etat )
  { if ( num<0 || num>=NBR_MESSAGE_ECRITS )
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "MSG : num %03d out of range", num );
       return;
     }

    if ( Partage->g[num].etat != etat )
     { Partage->g[num].etat = etat;

       if ( Partage->g[num].last_change + 10 <= Partage->top )   /* Si pas de change depuis plus de 1 sec */
        { Partage->g[num].changes = 0; }

       if ( Partage->g[num].changes <= 5 ) 
        { pthread_mutex_lock( &Partage->com_msrv.synchro );       /* Ajout dans la liste de msg a traiter */
          if (etat) 
           { Partage->com_msrv.liste_msg_on  = g_slist_append( Partage->com_msrv.liste_msg_on,
                                                               GINT_TO_POINTER(num) );
           }
          else
           { Partage->com_msrv.liste_msg_off = g_slist_append( Partage->com_msrv.liste_msg_off,
                                                               GINT_TO_POINTER(num) );
           }
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Partage->g[num].changes++;
        } else if ( ! (Partage->top % 600) )                          /* On previent toutes les minutes ! */
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "MSG: last_change trop tot !", num ); }
       Partage->g[num].last_change = Partage->top;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* Envoyer_commande_dls: Gestion des envois de commande DLS                                               */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_commande_dls ( int num )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.liste_m = g_list_append ( Partage->com_dls.liste_m, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
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
     { gint num;
       num = GPOINTER_TO_INT(liste->data);
       Info_new( Config.log, Config.log_all, LOG_INFO, "Raz_cde_exterieure : Mise a zero du bit M%03d", num );
       SM( num, 0 );
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

    Partage->com_dls.Plugins            = NULL;                 /* Initialisation des variables du thread */
    Partage->com_dls.liste_m            = NULL;
    Partage->com_dls.liste_plugin_reset = NULL;
    Partage->com_dls.Thread_run         = TRUE;                                     /* Le thread tourne ! */

    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                   /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                            /* Active le timer */

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_dls: Demarrage" );                /* Log Start */
             
    Prendre_heure();                                 /* On initialise les variables de gestion de l'heure */
    sleep(30);/* attente 30 secondes pour initialisation des bit internes et collection des infos modules */
    Charger_plugins();                                                      /* Chargement des modules dls */

    while(Partage->com_dls.Thread_run == TRUE)                           /* On tourne tant que necessaire */
     { struct timeval tv_avant, tv_apres;

       if (Partage->com_dls.Thread_reload)
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_dls: RELOADING" );
          Decharger_plugins();
          Charger_plugins();
          Partage->com_dls.Thread_reload = FALSE;
        }

       if (Partage->com_dls.Thread_sigusr1)
        { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_dls: SIGUSR1" );
          Partage->com_dls.Thread_sigusr1 = FALSE;
        }

       if (Partage->top-Update_heure>=600)      /* Gestion des changements d'horaire (toutes les minutes) */
        { Prendre_heure ();                            /* Mise à jour des variables de gestion de l'heure */
          Update_heure=Partage->top;
        }

       if (Partage->com_dls.liste_m)                                 /* A-t-on un monostable a allumer ?? */
        { gint num;
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_m->data );
          Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_dls: Mise a un du bit M%03d", num );
          Partage->com_dls.liste_m = g_list_remove ( Partage->com_dls.liste_m, GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          SM( num, 1 );                                                    /* Mise a un du bit monostable */
          Cde_exterieure = g_list_append( Cde_exterieure, GINT_TO_POINTER( num ) ); 
        }

       if (Partage->com_dls.admin_start)                                  /* A-t-on un plugin a allumer ? */
        { Activer_plugin_by_id ( Partage->com_dls.admin_start, TRUE );
          Partage->com_dls.admin_start = 0;
        }

       if (Partage->com_dls.admin_stop)                                  /* A-t-on un plugin a eteindre ? */
        { Activer_plugin_by_id ( Partage->com_dls.admin_stop, FALSE );
          Partage->com_dls.admin_stop = 0;
        }

       if (Partage->com_dls.liste_plugin_reset)                          /* A-t-on un plugin a reseter ?? */
        { gint num;
          pthread_mutex_lock( &Partage->com_dls.synchro );
          num = GPOINTER_TO_INT( Partage->com_dls.liste_plugin_reset->data );
          Partage->com_dls.liste_plugin_reset = g_list_remove ( Partage->com_dls.liste_plugin_reset,
                                                                GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Reseter_un_plugin( num );
        }

       SB(0, !B(0));                                            /* Change d'etat tous les tours programme */
       SB(1, 0);                                                                   /* B1 est toujours à 0 */
       SB(2, 1);                                                                   /* B2 est toujours à 1 */
       SI(1, 1, 255, 0, 0, 0 );                                           /* Icone toujours à 1:rouge */

       pthread_mutex_lock( &Partage->com_dls.synchro );
       plugins = Partage->com_dls.Plugins;
       while(plugins)                                            /* On execute tous les modules un par un */
        { struct PLUGIN_DLS *plugin_actuel;
          plugin_actuel = (struct PLUGIN_DLS *)plugins->data;

          if (plugin_actuel->plugindb.on && plugin_actuel->go)
           { gettimeofday( &tv_avant, NULL );
             Partage->top_cdg_plugin_dls = 0;                               /* On reset le cdg plugin DLS */
             plugin_actuel->go( plugin_actuel->starting );                          /* On appel le plugin */
             gettimeofday( &tv_apres, NULL );
             plugin_actuel->conso+=Chrono( &tv_avant, &tv_apres );
             plugin_actuel->starting = 0;
           }
          plugins = plugins->next;
        }
       pthread_mutex_unlock( &Partage->com_dls.synchro );
       SB(3, 1);                                  /* B3 est toujours à un apres le premier tour programme */

       Raz_cde_exterieure();                        /* Mise à zero des monostables de commande exterieure */
       Partage->audit_tour_dls_per_sec++;               /* Gestion de l'audit nbr de tour DLS par seconde */
/************************************ Gestion des 1000 tours DLS par seconde ******************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Decharger_plugins();                                                  /* Dechargement des modules DLS */
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_dls: DLS Down (%d)", pthread_self() );
    Partage->com_dls.TID = 0;                             /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
