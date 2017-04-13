/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Chrono: renvoi la difference de temps entre deux structures timeval                                                        */
/* Entrée: le temps avant, et le temps apres l'action                                                                         */
/* Sortie: un float                                                                                                           */
/******************************************************************************************************************************/
 static float Chrono ( struct timeval *avant, struct timeval *apres )
  { if (!(avant && apres)) return(0.0);
    else return( apres->tv_sec - avant->tv_sec + (apres->tv_usec - avant->tv_usec)/1000000.0 );
  }

/******************************************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                                          */
/******************************************************************************************************************************/
 int E( int num )
  { if ( (num>=0) && (num<NBR_ENTRE_TOR) ) return ( (Partage->e[ num ].etat ? 1 : 0) );
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "E : num %d out of range", num ); }
     }
    return(0);
  }
/******************************************************************************************************************************/
/* SE : Met à jour la valeur de l'entrée en parametre.                                                                        */
/* Utilisé directement par les threads locaux, via Envoyer_entree_furtive_dls pour les evenements                             */
/******************************************************************************************************************************/
 void SE( int num, int etat )
  { if ( (E(num) && !etat) || (!E(num) && etat) )
     { Ajouter_arch( MNEMO_ENTREE, num, 1.0*E(num) );                       /* Archivage etat n-1 pour les courbes historique */
       Ajouter_arch( MNEMO_ENTREE, num, 1.0*etat );                                            /* Archivage de l'etat courant */
       Partage->e[num].etat = etat;                                                          /* Changement d'etat de l'entrée */
     }
  }
/******************************************************************************************************************************/
/* R : Renvoie la valeur du registre dont le numéro est en parametre                                                          */
/******************************************************************************************************************************/
 float R( int num )
  { if (num>=0 && num<NBR_REGISTRE)
     { return ( Partage->registre[ num ].val );
     }
    if (!(Partage->top % 600))
     { Info_new( Config.log, Config.log_dls, LOG_INFO, "%s : num %d out of range", __func__, num ); }
    return(0.0);
  }
/******************************************************************************************************************************/
/* SR : Positionne la valeur du registre dont le numéro est en parametre                                                      */
/******************************************************************************************************************************/
 void SR( int num, float val )
  { if (num>=0 && num<NBR_REGISTRE)
     { Partage->registre[ num ].val = val; }
    else if (!(Partage->top % 600))
     { Info_new( Config.log, Config.log_dls, LOG_INFO, "%s : num %d out of range", __func__, num ); }
  }
/******************************************************************************************************************************/
/* EA_inrange : Renvoie 1 si l'EA en paramètre est dans le range de mesure                                                    */
/******************************************************************************************************************************/
 int EA_inrange( int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) return( Partage->ea[ num ].inrange);
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_range : num %d out of range", num ); }
     }
    return(0);
  }
/******************************************************************************************************************************/
/* EA_ech : Renvoie la valeur de l'EA interprétée (mis à l'échelle)                                                           */
/******************************************************************************************************************************/
 float EA_ech( int num )
  { if (num>=0 && num<NBR_ENTRE_ANA)
     { gfloat val_ech;
       val_ech = Partage->ea[ num ].val_ech;
       return ( val_ech );
     }
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_ech : num %d out of range", num ); }
     }
    return(0.0);
  }
/******************************************************************************************************************************/
/* EA_ech_inf : Teste si la valeur de l'EA est inf à une mesure                                                               */
/******************************************************************************************************************************/
 int EA_ech_inf( float val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) < val); }
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_ech_inf : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_inf_egal : Teste si la valeur de l'EA est inf ou egale à une mesure                             */
/**********************************************************************************************************/
 int EA_ech_inf_egal( float val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) <= val); }
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_ech_inf_egal : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_sup : Teste si la valeur de l'EA est sup à une mesure                                           */
/**********************************************************************************************************/
 int EA_ech_sup( float val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) > val); }
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_ech_sup : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* EA_ech_sup_egal : Teste si la valeur de l'EA est sup ou egale à une mesure                             */
/**********************************************************************************************************/
 int EA_ech_sup_egal( float val, int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) { if (EA_inrange(num)) return (EA_ech(num) >= val); }
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "EA_ech_sup_egal : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 float CI( int num )
  { if (num<NBR_COMPTEUR_IMP) return (Partage->ci[ num ].confDB.valeur);
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "CI : num %d out of range", num ); }
     }
    return(0.0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une entre TOR                                                                      */
/**********************************************************************************************************/
 int A( int num )
  { if ( num>=0 && num<NBR_SORTIE_TOR ) return ( Partage->a[ num ].etat );
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "A : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'un bistable                                                                        */
/**********************************************************************************************************/
 int B( int num )
  { if (num>=0 && num<NBR_BIT_BISTABLE) return( ((Partage->b[ num>>3 ]) & (1<<(num%8)) ? 1 : 0 ) );
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "B : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'un monostable                                                                      */
/**********************************************************************************************************/
 int M( int num )
  { if (num>=0 && num<NBR_BIT_MONOSTABLE) return( ((Partage->m[ num>>3 ]) & (1<<(num%8)) ? 1 : 0 ) );
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "M : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Renvoie la valeur d'une tempo retard                                                                   */
/**********************************************************************************************************/
 int T( int num )
  { if (num>=0 && num<NBR_TEMPO) return ( Partage->Tempo_R[num].state );
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "TR : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num    val_avant_ech sur 12 bits !!                                     */
/**********************************************************************************************************/
 void SEA_range( int num, int range )
  { if (num<0 || num>=NBR_ENTRE_ANA)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SEA_range : num %d out of range", num ); }
       return;
     }
    Partage->ea[num].inrange = range;
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                        */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 void SEA ( int num, float val_avant_ech )
  { gboolean need_arch;
    if (num<0 || num>=NBR_ENTRE_ANA)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SEA : num %d out of range", num ); }
       return;
     }

    need_arch = FALSE;
    if (Partage->ea[ num ].val_avant_ech != val_avant_ech)
     { Partage->ea[ num ].val_avant_ech = val_avant_ech;        /* Archive au mieux toutes les 5 secondes */
       if ( Partage->ea[ num ].last_arch + ARCHIVE_EA_TEMPS_SI_VARIABLE < Partage->top )
        { need_arch = TRUE; }

       switch ( Partage->ea[num].confDB.type )
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
                  Partage->ea[ num ].val_ech = (gfloat)
                  ((val_avant_ech-204)*(Partage->ea[num].confDB.max - Partage->ea[num].confDB.min))/820.0
                  + Partage->ea[num].confDB.min;                                    /* Valeur à l'echelle */ 

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
                  Partage->ea[ num ].val_ech = (gfloat)
                  ((val_avant_ech-816)*(Partage->ea[num].confDB.max - Partage->ea[num].confDB.min))/3280.0
                     + Partage->ea[num].confDB.min;                          /* Valeur à l'echelle */ 
                  Partage->ea[ num ].inrange = 1;
                }
               break;
          case ENTREEANA_WAGO_750455:                                                                              /* 4/20 mA */
               Partage->ea[ num ].val_ech = (gfloat)
                  (val_avant_ech*(Partage->ea[num].confDB.max - Partage->ea[num].confDB.min))/4095.0
                     + Partage->ea[num].confDB.min;                          /* Valeur à l'echelle */ 
               Partage->ea[ num ].inrange = 1;
               break;
          case ENTREEANA_WAGO_750461:                                                                          /* Borne PT100 */
               if (val_avant_ech > -32767 && val_avant_ech < 8500)
                { Partage->ea[ num ].val_ech = (gfloat)(val_avant_ech/10.0);                         /* Valeur à l'echelle */ 
                  Partage->ea[ num ].inrange = 1;
                }
               else Partage->ea[ num ].inrange = 0;
               break;
          default:
               Partage->ea[ num ].val_ech = 0.0;
               Partage->ea[ num ].inrange = 0;
        }
     }
    else if ( Partage->ea[ num ].last_arch + ARCHIVE_EA_TEMPS_SI_CONSTANT < Partage->top )
     { need_arch = TRUE; }                                           /* Archive au pire toutes les 10 min */

    if (need_arch)
     { Ajouter_arch( MNEMO_ENTREE_ANA, num, Partage->ea[num].val_ech );            /* Archivage si besoin */
       Partage->ea[ num ].last_arch = Partage->top;                       /* Communications aux threads ! */
     }
  }
/******************************************************************************************************************************/
/* SB_SYS: Positionnement d'un bistable DLS sans controle sur le range reserved                                               */
/* Entrée: numero, etat                                                                                                       */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 void SB_SYS( int num, int etat )
  { gint numero, bit;
    if (num<0 || num>=NBR_BIT_BISTABLE)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SB : num %d out of range", num ); }
       return;
     }
    numero = num>>3;
    bit = 1<<(num & 0x07);
    if (etat)                                                                                           /* Mise a jour du bit */
     { Partage->b[numero] |= bit; 
       Partage->audit_bit_interne_per_sec++;
     }
    else
     { Partage->b[numero] &= ~bit;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* SB: Positionnement d'un bistable DLS avec controle sur le range reserved                                                   */
/* Entrée: numero, etat                                                                                                       */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 void SB( int num, int etat )
  { if (num<=NBR_BIT_BISTABLE_RESERVED || num>=NBR_BIT_BISTABLE)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SB : num %d out of range", num ); }
       return;
     }
    SB_SYS( num, etat );
  }
/**********************************************************************************************************/
/* SM: Positionnement d'un monostable DLS                                                                 */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void SM( int num, int etat )
  { gint numero, bit;
    if (num<0 || num>=NBR_BIT_MONOSTABLE)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SM : num %d out of range", num ); }
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
  { if ( num<0 || num>=NBR_BIT_CONTROLE )
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SI : num %d out of range", num ); }
       return;
     }

    if (Partage->i[num].etat   != etat || Partage->i[num].rouge != rouge || 
        Partage->i[num].vert   != vert || Partage->i[num].bleu  != bleu  ||
        Partage->i[num].cligno != cligno
       )
     { if ( Partage->i[num].last_change + 50 <= Partage->top )   /* Si pas de change depuis plus de 5 sec */
        { Partage->i[num].changes = 0; }

       if ( Partage->i[num].changes <= 10 )                            /* Si moins de 10 changes en 5 sec */
        {
          if ( Partage->i[num].changes == 10 )                  /* Est-ce le dernier change avant blocage */
           { Partage->i[num].etat   = 0;                     /* Si oui, on passe le visuel en kaki cligno */
             Partage->i[num].rouge  = 0;
             Partage->i[num].vert   = 100;                                                   /* Mode Kaki */
             Partage->i[num].bleu   = 0;
             Partage->i[num].cligno = 1;                                                    /* Clignotant */
           }
          else { Partage->i[num].etat   = etat;  /* Sinon on recopie ce qui est demandé par le plugin DLS */
                 Partage->i[num].rouge  = rouge;
                 Partage->i[num].vert   = vert;
                 Partage->i[num].bleu   = bleu;
                 Partage->i[num].cligno = cligno;
               }

          Partage->i[num].last_change = Partage->top;                               /* Date de la photo ! */
          pthread_mutex_lock( &Partage->com_msrv.synchro );         /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_i = g_slist_append( Partage->com_msrv.liste_i,
                                                      GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Partage->i[num].changes++;                                               /* Un change de plus ! */
        }
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* STR: Positionnement d'une Tempo retard DLS                                                             */
/* Entrée: numero, etat                                                                                   */
/* Sortie: Neant                                                                                          */
/**********************************************************************************************************/
 void ST( int num, int etat )
  { struct TEMPO *tempo;

    if (num<0 || num>=NBR_TEMPO)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "STR: num %d out of range", num ); }
       return;
     }
    tempo = &Partage->Tempo_R[num];                                       /* Récupération de la structure */

    if (tempo->status == TEMPO_NOT_COUNTING && etat == 1)
     { tempo->status = TEMPO_WAIT_FOR_DELAI_ON;
       tempo->date_on = Partage->top + tempo->confDB.delai_on;
     }

    if (tempo->status == TEMPO_WAIT_FOR_DELAI_ON && etat == 0)
     { tempo->status = TEMPO_NOT_COUNTING; }

    if (tempo->status == TEMPO_WAIT_FOR_DELAI_ON && tempo->date_on <= Partage->top)
     { tempo->status = TEMPO_WAIT_FOR_MIN_ON;
       tempo->state = TRUE;
     }

    if (tempo->status == TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        Partage->top < tempo->date_on + tempo->confDB.min_on )
     { if (Partage->top+tempo->confDB.delai_off <= tempo->date_on + tempo->confDB.min_on)
            { tempo->date_off = tempo->date_on+tempo->confDB.min_on; }
       else { tempo->date_off = Partage->top+tempo->confDB.delai_off; }
       tempo->status = TEMPO_WAIT_FOR_DELAI_OFF;
     }
    
    if (tempo->status == TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        tempo->date_on + tempo->confDB.min_on <= Partage->top )
     { tempo->date_off = Partage->top+tempo->confDB.delai_off;
       tempo->status = TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == TEMPO_WAIT_FOR_MIN_ON && etat == 1 &&
        tempo->date_on + tempo->confDB.min_on <= Partage->top )
     { tempo->status = TEMPO_WAIT_FOR_MAX_ON;
     }

    if (tempo->status == TEMPO_WAIT_FOR_MAX_ON && etat == 0 )
     { if (tempo->confDB.max_on)
            { if (Partage->top+tempo->confDB.delai_off < tempo->date_on+tempo->confDB.max_on)
                   { tempo->date_off = Partage->top + tempo->confDB.delai_off; }
              else { tempo->date_off = tempo->date_on+tempo->confDB.max_on; }
            }
       else { tempo->date_off = Partage->top+tempo->confDB.delai_off; }
       tempo->status = TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == TEMPO_WAIT_FOR_MAX_ON && etat == 1 && tempo->confDB.max_on &&
        tempo->date_on + tempo->confDB.max_on <= Partage->top )
     { tempo->date_off = tempo->date_on+tempo->confDB.max_on;
       tempo->status = TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == TEMPO_WAIT_FOR_DELAI_OFF && tempo->date_off <= Partage->top )
     { tempo->date_on = tempo->date_off = 0;
       tempo->status = TEMPO_WAIT_FOR_COND_OFF;
       tempo->state = FALSE;
     }

    if (tempo->status == TEMPO_WAIT_FOR_COND_OFF && etat == 0 )
     { tempo->status = TEMPO_NOT_COUNTING; }
  }
/******************************************************************************************************************************/
/* SA: Positionnement d'un actionneur DLS                                                                                     */
/* Entrée: numero, etat                                                                                                       */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 void SA( int num, int etat )
  { if (num<0 || num>=NBR_SORTIE_TOR)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SA : num %d out of range", num ); }
       return;
     }

    if ( Partage->a[num].etat != etat )
     { Partage->a[num].etat = etat;

       if ( Partage->a[num].last_change + 10 <= Partage->top )                       /* Si pas de change depuis plus de 1 sec */
        { Partage->a[num].changes = 0; }

       if ( Partage->a[num].changes <= 5 )                    /* Arbitraire : si plus de 5 changes dans la seconde, on bloque */
        { Ajouter_arch( MNEMO_SORTIE, num, !etat );                           /* Sauvegarde etat n-1 pour courbes historiques */
          Ajouter_arch( MNEMO_SORTIE, num, 1.0*etat );                                              /* Sauvegarde de l'etat n */
          if (etat == 1)
           { pthread_mutex_lock( &Partage->com_msrv.synchro );                  /* Ajout dans la liste des Events A a traiter */
             Partage->com_msrv.liste_a = g_slist_append( Partage->com_msrv.liste_a,
                                                         GINT_TO_POINTER(num) );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          Partage->a[num].changes++;                                                                  /* Un change de plus !! */
        } else if ( ! (Partage->top % 50 ))                                    /* Si persistence on prévient toutes les 5 sec */
        { Info_new( Config.log, Config.log_dls, LOG_INFO, "SA: last_change trop tot pour A%d !", num ); }
       Partage->a[num].last_change = Partage->top;
       Partage->audit_bit_interne_per_sec++;
     }
  }
/**********************************************************************************************************/
/* Met à jour le compteur horaire                                                                         */
/* Le compteur compte les MINUTES !!                                                                      */
/**********************************************************************************************************/
 void SCH( int num, int etat, int reset )
  { if (num<0 || num>=NBR_COMPTEUR_H)
     { Info_new( Config.log, Config.log_dls, LOG_INFO, "SCH : num %d out of range", num );
       return;
     }
    if (etat)
     { if (reset)                                                   /* Le compteur doit-il etre resetté ? */
        { Partage->ch[num].confDB.valeur = 0; }

       if ( ! Partage->ch[ num ].actif )
        { Partage->ch[num].actif = TRUE;
          Partage->ch[num].old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - Partage->ch[num].old_top;
          if (delta > 600)                                           /* On compte +1 toutes les minutes ! */
           { Partage->ch[num].confDB.valeur ++;
             Partage->ch[num].old_top = new_top;
             Ajouter_arch( MNEMO_CPTH, num, 1.0*Partage->ch[num].confDB.valeur );
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
  { gboolean changed = FALSE;
    if (num<0 || num>=NBR_COMPTEUR_IMP)
     { Info_new( Config.log, Config.log_dls, LOG_INFO, "CI : num %d out of range", num );
       return;
     }
    if (etat)
     { if (reset)                                                   /* Le compteur doit-il etre resetté ? */
        { Partage->ci[num].val_en_cours1 = 0.0;                /* Valeur transitoire pour gérer les ratio */
          Partage->ci[num].val_en_cours2 = 0.0;                /* Valeur transitoire pour gérer les ratio */
          changed = TRUE;
        }
       else if ( ! Partage->ci[ num ].actif )                                         /* Passage en actif */
        { Partage->ci[num].actif = TRUE;
          Partage->ci[num].val_en_cours1++;
          if (Partage->ci[num].val_en_cours1>=ratio)
           { Partage->ci[num].val_en_cours2++;
             Partage->ci[num].val_en_cours1=0.0;                          /* RAZ de la valeur de calcul 1 */
             changed = TRUE;
           }
        }
     }
    else
     { if (reset==0) Partage->ci[ num ].actif = FALSE; }

    switch (Partage->ci[ num ].confDB.type)                        /* Calcul de la valeur réelle du CI */
     { case CI_TOTALISATEUR :
            if ( Partage->ci[num].last_update + 1 < Partage->top )
             { Partage->ci[num].confDB.valeur = Partage->ci[num].val_en_cours2;
               Partage->ci[num].last_update = Partage->top;
             }
            break;
       case CI_MOYENNEUR_SEC:
            if ( Partage->ci[num].last_update + 10 < Partage->top )
             { Partage->ci[num].confDB.valeur = (Partage->ci[num].confDB.valeur +
                                                    Partage->ci[num].val_en_cours2)/2.0;
               Partage->ci[num].val_en_cours2 = 0.0;
               Partage->ci[num].last_update = Partage->top;
             }
            break;
       case CI_MOYENNEUR_MIN:
            if ( Partage->ci[num].last_update + 600 < Partage->top )
             { Partage->ci[num].confDB.valeur = (Partage->ci[num].confDB.valeur +
                                                    Partage->ci[num].val_en_cours2)/2.0;
               Partage->ci[num].val_en_cours2 = 0.0;
               Partage->ci[num].last_update = Partage->top;
             }
            break;
     }
    if (changed == TRUE) Ajouter_arch( MNEMO_CPT_IMP, num, Partage->ci[num].confDB.valeur );
  }
/******************************************************************************************************************************/
/* MSG: Positionnement des messages DLS                                                                                       */
/* Entrée: numero, etat                                                                                                       */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 void MSG( int num, int etat )
  { if ( num<0 || num>=NBR_MESSAGE_ECRITS )
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: num %03d out of range", __func__, num ); }
       return;
     }

    if ( Partage->g[num].etat != etat )
     { Partage->g[num].etat = etat;

       if ( Partage->g[num].last_change + 10 <= Partage->top )                       /* Si pas de change depuis plus de 1 sec */
        { Partage->g[num].changes = 0; }

       if ( Partage->g[num].changes > 5 && !(Partage->top % 50) )   /* Si persistence d'anomalie on prévient toutes les 5 sec */
        { Info_new( Config.log, Config.log_dls, LOG_NOTICE, "%s: last_change trop tot for MSG%03d!", __func__, num ); }
       else if ( Partage->g[num].persist == FALSE)                    /* Si pas de persistence, on envoi l'evenement de suite */
        { struct MESSAGES_EVENT *event;
          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (!event)
           { Info_new( Config.log, Config.log_dls, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG%d", __func__, num );
           }
          else
           { event->num  = num;
             event->etat = etat;                                                        /* Recopie de l'état dans l'evenement */
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          Partage->g[num].changes++;
          Partage->g[num].last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
       else if (etat)                    /* Si persistence, le message persiste si etat = 0. Si etat=1, stop/start du message */
        { struct MESSAGES_EVENT *event;
          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (!event)
           { Info_new( Config.log, Config.log_dls, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG%d", __func__, num );
           }
          else
           { event->num  = num;
             event->etat = 0;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }

          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (!event)
           { Info_new( Config.log, Config.log_dls, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG%d", __func__, num );
           }
          else
           { event->num  = num;
             event->etat = 1;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          Partage->g[num].changes++;
          Partage->g[num].last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
     }
  }
/******************************************************************************************************************************/
/* Envoyer_commande_dls: Gestion des envois de commande DLS                                                                   */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_commande_dls ( int num )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Set_M = g_slist_append ( Partage->com_dls.Set_M, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_cde_exterieure ( void )
  { gint num;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    while( Partage->com_dls.Set_M )                                                      /* A-t-on un monostable a allumer ?? */
     { num = GPOINTER_TO_INT( Partage->com_dls.Set_M->data );
       Info_new( Config.log, Config.log_dls, LOG_NOTICE, "Set_cde_exterieure: Mise a un du bit M%03d", num );
       Partage->com_dls.Set_M   = g_slist_remove ( Partage->com_dls.Set_M,   GINT_TO_POINTER(num) );
       Partage->com_dls.Reset_M = g_slist_append ( Partage->com_dls.Reset_M, GINT_TO_POINTER(num) ); 
       SM( num, 1 );                                                                           /* Mise a un du bit monostable */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro ); 
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reset_cde_exterieure ( void )
  { gint num;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    while( Partage->com_dls.Reset_M )                                                                /* Reset des monostables */
     { num = GPOINTER_TO_INT(Partage->com_dls.Reset_M->data);
       Info_new( Config.log, Config.log_dls, LOG_INFO, "Reset_cde_exterieure : Mise a zero du bit M%03d", num );
       Partage->com_dls.Reset_M = g_slist_remove ( Partage->com_dls.Reset_M, GINT_TO_POINTER(num) );
       SM( num, 0 );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint Update_heure=0;
    GSList *plugins;

    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( Config.log, Config.log_dls, LOG_NOTICE,
              "Run_dls: Demarrage . . . TID = %p", pthread_self() );
    Partage->com_dls.Thread_run         = TRUE;                                                         /* Le thread tourne ! */
             
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */
    Charger_plugins();                                                                          /* Chargement des modules dls */
    SB_SYS(1, 0);                                                                                      /* B1 est toujours à 0 */
    SB_SYS(2, 1);                                                                                      /* B2 est toujours à 1 */
    sleep(30);                    /* attente 30 secondes pour initialisation des bit internes et collection des infos modules */

    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { struct timeval tv_avant, tv_apres;

       if (Partage->com_dls.Thread_reload)
        { Info_new( Config.log, Config.log_dls, LOG_NOTICE, "Run_dls: RELOADING" );
          Decharger_plugins();
          Charger_plugins();
          Partage->com_dls.Thread_reload = FALSE;
        }

       if (Partage->com_dls.Thread_sigusr1)
        { Info_new( Config.log, Config.log_dls, LOG_NOTICE, "Run_dls: SIGUSR1" );
          Partage->com_dls.Thread_sigusr1 = FALSE;
        }

       if (Partage->top-Update_heure>=600)      /* Gestion des changements d'horaire (toutes les minutes) */
        { Prendre_heure ();                            /* Mise à jour des variables de gestion de l'heure */
          Update_heure=Partage->top;
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
          Partage->com_dls.liste_plugin_reset = g_slist_remove ( Partage->com_dls.liste_plugin_reset,
                                                                 GINT_TO_POINTER(num) );
          pthread_mutex_unlock( &Partage->com_dls.synchro );
          Reseter_un_plugin( num );
        }

       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

       SB_SYS(0, !B(0));                                                            /* Change d'etat tous les tours programme */
       SI(1, 1, 255, 0, 0, 0 );                                                                   /* Icone toujours à 1:rouge */

       pthread_mutex_lock( &Partage->com_dls.synchro );
       plugins = Partage->com_dls.Plugins;
       while(plugins)                                                                /* On execute tous les modules un par un */
        { struct PLUGIN_DLS *plugin_actuel;
          plugin_actuel = (struct PLUGIN_DLS *)plugins->data;

          if (plugin_actuel->plugindb.on && plugin_actuel->go)
           { gettimeofday( &tv_avant, NULL );
             Partage->top_cdg_plugin_dls = 0;                                                   /* On reset le cdg plugin DLS */
             plugin_actuel->go( plugin_actuel->starting );                                              /* On appel le plugin */
             gettimeofday( &tv_apres, NULL );
             plugin_actuel->conso+=Chrono( &tv_avant, &tv_apres );
             plugin_actuel->starting = 0;
           }
          plugins = plugins->next;
        }
       pthread_mutex_unlock( &Partage->com_dls.synchro );
       SB_SYS(3, 1);                                                  /* B3 est toujours à un apres le premier tour programme */

       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Decharger_plugins();                                                                      /* Dechargement des modules DLS */
    Info_new( Config.log, Config.log_dls, LOG_NOTICE, "Run_dls: DLS Down (%p)", pthread_self() );
    Partage->com_dls.TID = 0;                                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
