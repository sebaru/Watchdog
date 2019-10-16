/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Dls_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Partage->com_dls.Compil_at_boot = FALSE;                                                   /* Settings default parameters */
    Partage->com_dls.Thread_debug   = FALSE;                                                   /* Settings default parameters */

    if ( ! Recuperer_configDB( &db, "dls" ) )                                               /* Connexion a la base de données */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Partage->com_dls.Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "compil_at_boot" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Partage->com_dls.Compil_at_boot = TRUE;  }
       else
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_print_debug: Affiche les valeurs des bits utilisés dans le plugin.                                                     */
/* Entrée: Appelé directement par le plugin, avec ne parametre les tableaux de bit et de valeur                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_print_debug ( gint id, gint *Tableau_bit, gint *Tableau_num, gfloat *Tableau_val )
  { gchar chaine[32], result[1024];
    gboolean change=FALSE;
    gint cpt, type;
    cpt=0;
    result[0]=0;
    while( (type=Tableau_bit[cpt]) != -1)
     { switch (type)
        { case MNEMO_SORTIE:
           { if (A(Tableau_num[cpt])!=Tableau_val[cpt])
              { g_snprintf( chaine, sizeof(chaine), "A[%04d]=%d, ", Tableau_num[cpt], A(Tableau_num[cpt]) );
                g_strlcat(result, chaine, sizeof(result));
                Tableau_val[cpt] = (float)A(Tableau_num[cpt]);
                change=TRUE;
              }
             break;
           }
          case MNEMO_MONOSTABLE:
           { if (M(Tableau_num[cpt])!=Tableau_val[cpt])
              { g_snprintf( chaine, sizeof(chaine), "M[%04d]=%d, ", Tableau_num[cpt], M(Tableau_num[cpt]) );
                g_strlcat(result, chaine, sizeof(result));
                Tableau_val[cpt] = (float)M(Tableau_num[cpt]);
                change=TRUE;
              }
             break;
           }
          case MNEMO_BISTABLE:
           { if (B(Tableau_num[cpt])!=Tableau_val[cpt])
              { g_snprintf( chaine, sizeof(chaine), "B[%04d]=%d, ", Tableau_num[cpt], B(Tableau_num[cpt]) );
                g_strlcat(result, chaine, sizeof(result));
                Tableau_val[cpt] = (float)B(Tableau_num[cpt]);
                change=TRUE;
              }
             break;
           }
/*          case MNEMO_CPTH:
           { if (CH(Tableau_num[cpt])!=Tableau_val[cpt])
              { g_snprintf( chaine, sizeof(chaine), "CH[%04d]=%d, ", Tableau_num[cpt], CH(Tableau_num[cpt]) );
                g_strlcat(result, chaine, sizeof(result));
                Tableau_val[cpt] = (float)CH(Tableau_num[cpt]);
                change=TRUE;
              }
             break;
           }*/
          case MNEMO_CPT_IMP:
           { if (CI(Tableau_num[cpt])!=Tableau_val[cpt])
              { g_snprintf( chaine, sizeof(chaine), "CI[%04d]=%f, ", Tableau_num[cpt], CI(Tableau_num[cpt]) );
                g_strlcat(result, chaine, sizeof(result));
                Tableau_val[cpt] = (float)CI(Tableau_num[cpt]);
                change=TRUE;
              }
             break;
           }
        }
       cpt++;
     }
   if (change)
    { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : DLS[%06d]->%s", __func__, id, result ); }
 }
/******************************************************************************************************************************/
/* Dls_get_top_alerte: Remonte la valeur du plus haut bit d'alerte dans l'arbre DLS                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: TRUE ou FALSe                                                                                                      */
/******************************************************************************************************************************/
 gboolean Dls_get_top_alerte ( void )
  { return( Partage->com_dls.Dls_tree->syn_vars.bit_alerte ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "E : num %d out of range", num ); }
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
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : num %d out of range", __func__, num ); }
    return(0.0);
  }
/******************************************************************************************************************************/
/* SR : Positionne la valeur du registre dont le numéro est en parametre                                                      */
/******************************************************************************************************************************/
 void SR( int num, float val )
  { if (num>=0 && num<NBR_REGISTRE)
     { Partage->registre[ num ].val = val; }
    else if (!(Partage->top % 600))
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : num %d out of range", __func__, num ); }
  }
/******************************************************************************************************************************/
/* EA_inrange : Renvoie 1 si l'EA en paramètre est dans le range de mesure                                                    */
/******************************************************************************************************************************/
 int EA_inrange( int num )
  { if (num>=0 && num<NBR_ENTRE_ANA) return( Partage->ea[ num ].inrange);
    else
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_range : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_ech : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_ech_inf : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_ech_inf_egal : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_ech_sup : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "EA_ech_sup_egal : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "CI : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "A : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "B : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "M : num %d out of range", num ); }
     }
    return(0);
  }
/**********************************************************************************************************/
/* Met à jour l'entrée analogique num    val_avant_ech sur 12 bits !!                                     */
/**********************************************************************************************************/
 void SEA_range( int num, int range )
  { if (num<0 || num>=NBR_ENTRE_ANA)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SEA_range : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SEA : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SB : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SB : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SM : num %d out of range", num ); }
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SI : num %d out of range", num ); }
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
/******************************************************************************************************************************/
/* STR_local: Positionnement d'une Tempo retard DLS                                                                           */
/* Entrée: la structure tempo et son etat                                                                                     */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 static void ST_local( struct DLS_TEMPO *tempo, int etat )
  { static guint seed;
    if (tempo->status == DLS_TEMPO_NOT_COUNTING && etat == 1)
     { tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_ON;
       if (tempo->random)
        { gfloat ratio;
          ratio = (gfloat)rand_r(&seed)/RAND_MAX;
          tempo->delai_on  = (gint)(tempo->random * ratio);
          if (tempo->delai_on<10) tempo->delai_on = 10;
          tempo->min_on    = 0;
          tempo->max_on    = 0;
          tempo->delai_off = 0;
        }
       tempo->date_on = Partage->top + tempo->delai_on;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && etat == 0)
     { tempo->status = DLS_TEMPO_NOT_COUNTING; }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && tempo->date_on <= Partage->top)
     { tempo->status = DLS_TEMPO_WAIT_FOR_MIN_ON;
       tempo->state = TRUE;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        Partage->top < tempo->date_on + tempo->min_on )
     { if (Partage->top+tempo->delai_off <= tempo->date_on + tempo->min_on)
            { tempo->date_off = tempo->date_on+tempo->min_on; }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->date_off = Partage->top+tempo->delai_off;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 1 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->status = DLS_TEMPO_WAIT_FOR_MAX_ON;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 0 )
     { if (tempo->max_on)
            { if (Partage->top+tempo->delai_off < tempo->date_on+tempo->max_on)
                   { tempo->date_off = Partage->top + tempo->delai_off; }
              else { tempo->date_off = tempo->date_on+tempo->max_on; }
            }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 1 && tempo->max_on &&
        tempo->date_on + tempo->max_on <= Partage->top )
     { tempo->date_off = tempo->date_on+tempo->max_on;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_OFF && tempo->date_off <= Partage->top )
     { tempo->date_on = tempo->date_off = 0;
       tempo->status = DLS_TEMPO_WAIT_FOR_COND_OFF;
       tempo->state = FALSE;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_COND_OFF && etat == 0 )
     { tempo->status = DLS_TEMPO_NOT_COUNTING; }
  }
/******************************************************************************************************************************/
/* SA: Positionnement d'un actionneur DLS                                                                                     */
/* Entrée: numero, etat                                                                                                       */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 void SA( int num, int etat )
  { if (num<0 || num>=NBR_SORTIE_TOR)
     { if (!(Partage->top % 600))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SA : num %d out of range", num ); }
       return;
     }

    if ( Partage->a[num].etat != etat )
     { Partage->a[num].etat = etat;

       if ( Partage->top <= Partage->a[num].last_change + 3 )        /* Si changement en moins de 3 dizieme depuis le dernier */
        { if ( ! (Partage->top % 50 ))                                         /* Si persistence on prévient toutes les 5 sec */
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: last_change trop tot pour A%d !", __func__, num ); }
        }
       else
        { Ajouter_arch( MNEMO_SORTIE, num, 1.0*etat );                                              /* Sauvegarde de l'etat n */
        }
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
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "SCH : num %d out of range", num );
       return;
     }

    if (reset)
     { if (etat)
        { Partage->ch[num].confDB.valeur = 0;
          Partage->ch[num].actif = FALSE;
        }
     }
    else if (etat)
     { if ( ! Partage->ch[ num ].actif )
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
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "CI : num %d out of range", num );
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
         { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: num %03d out of range", __func__, num ); }
        return;
      }

    if ( Partage->g[num].etat != etat )
     { Partage->g[num].etat = etat;

       if ( Partage->g[num].last_change + 10 <= Partage->top )                       /* Si pas de change depuis plus de 1 sec */
        { Partage->g[num].changes = 0; }

       if ( Partage->g[num].changes > 5 && !(Partage->top % 50) )   /* Si persistence d'anomalie on prévient toutes les 5 sec */
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: last_change trop tot for MSG%03d!", __func__, num ); }
       else if ( Partage->g[num].persist == FALSE)                    /* Si pas de persistence, on envoi l'evenement de suite */
        { struct MESSAGES_EVENT *event;
          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
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
/* Envoyer_commande_dls_data: Gestion des envois de commande DLS via dls_data                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme )
  { gpointer bool=NULL;
    Dls_data_get_bool ( tech_id, acronyme, &bool );
    if (!bool) { Dls_data_set_bool ( tech_id, acronyme, &bool, TRUE ); }

    pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Set_Dls_Data = g_slist_append ( Partage->com_dls.Set_Dls_Data, bool );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a un du bit '%s:%s' demandée", __func__, tech_id, acronyme );
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
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a un du bit M%03d", __func__, num );
       Partage->com_dls.Set_M   = g_slist_remove ( Partage->com_dls.Set_M,   GINT_TO_POINTER(num) );
       Partage->com_dls.Reset_M = g_slist_append ( Partage->com_dls.Reset_M, GINT_TO_POINTER(num) );
       SM( num, 1 );                                                                           /* Mise a un du bit monostable */
     }
    while( Partage->com_dls.Set_Dls_Data )                                               /* A-t-on un monostable a allumer ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Set_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a 1 du bit %s:%s", __func__, bool->tech_id, bool->acronyme );
       Partage->com_dls.Set_Dls_Data = g_slist_remove ( Partage->com_dls.Set_Dls_Data, bool );
       Partage->com_dls.Reset_Dls_Data = g_slist_append ( Partage->com_dls.Reset_Dls_Data, bool );
       Dls_data_set_bool ( NULL, NULL, (gpointer *)&bool, TRUE );                              /* Mise a un du bit monostable */
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
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Mise a zero du bit M%03d", __func__, num );
       Partage->com_dls.Reset_M = g_slist_remove ( Partage->com_dls.Reset_M, GINT_TO_POINTER(num) );
       SM( num, 0 );
     }
    while( Partage->com_dls.Reset_Dls_Data )                                            /* A-t-on un monostable a éteindre ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Reset_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Mise a 0 du bit %s:%s", __func__, bool->tech_id, bool->acronyme );
       Partage->com_dls.Reset_Dls_Data = g_slist_remove ( Partage->com_dls.Reset_Dls_Data, bool );
       Dls_data_set_bool ( NULL, NULL, (gpointer *)&bool, FALSE );                             /* Mise a un du bit monostable */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/* Dls_data_set_bool: Positionne un boolean                                                                                   */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_bool ( gchar *tech_id, gchar *acronyme, gpointer *bool_p, gboolean valeur )
  { struct DLS_BOOL *bool;

    if (!bool_p || !*bool_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_BOOL;
       while (liste)
        { bool = (struct DLS_BOOL *)liste->data;
          if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { bool = g_try_malloc0 ( sizeof(struct DLS_BOOL) );
          if (!bool)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( bool->acronyme, sizeof(bool->acronyme), "%s", acronyme );
          g_snprintf( bool->tech_id,  sizeof(bool->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_BOOL = g_slist_prepend ( Partage->Dls_data_BOOL, bool );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding DLS_BOOL '%s:%s'", __func__, tech_id, acronyme );
        }
       if (bool_p) *bool_p = (gpointer)bool;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else bool = (struct DLS_BOOL *)*bool_p;

    if (valeur == TRUE && bool->etat==FALSE) { bool->edge_up = TRUE; } else { bool->edge_up = FALSE; }
    if (valeur == FALSE && bool->etat==TRUE) { bool->edge_down = TRUE; } else { bool->edge_down = FALSE; }
    if (bool->etat != valeur)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : Changing DLS_BOOL '%s:%s'=%d up %d down %d",
                 __func__, bool->tech_id, bool->acronyme, valeur, bool->edge_up, bool->edge_down );
     }
    bool->etat = valeur;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool: Remonte l'etat d'un boolean                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool_up ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->edge_up );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool_down ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->edge_down );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_data_set_bool: Positionne un boolean                                                                                   */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_DO ( gchar *tech_id, gchar *acronyme, gpointer *bool_p, gboolean valeur )
  { struct DLS_BOOL *bool;
    Dls_data_set_bool ( tech_id, acronyme, bool_p, valeur );
    bool = *bool_p;

    if ( bool->edge_up )
     { pthread_mutex_lock( &Partage->com_msrv.synchro );
       Partage->com_msrv.Liste_DO = g_slist_prepend ( Partage->com_msrv.Liste_DO, bool );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
     }
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, float val_avant_ech )
  { struct DLS_AI *ai;
    gboolean need_arch;

    if (!ai_p || !*ai_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_AI;
       while (liste)
        { ai = (struct DLS_AI *)liste->data;
          if ( !strcasecmp ( ai->acronyme, acronyme ) && !strcasecmp( ai->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { ai = g_try_malloc0 ( sizeof(struct DLS_AI) );
          if (!ai)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( ai->acronyme, sizeof(ai->acronyme), "%s", acronyme );
          g_snprintf( ai->tech_id,  sizeof(ai->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_AI = g_slist_prepend ( Partage->Dls_data_AI, ai );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding AI '%s:%s'", __func__, tech_id, acronyme );
        }
       if (ai_p) *ai_p = (gpointer)ai;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else ai = (struct DLS_AI *)*ai_p;

    need_arch = FALSE;
    if (ai->val_avant_ech != val_avant_ech)
     { ai->val_avant_ech = val_avant_ech;                                           /* Archive au mieux toutes les 5 secondes */
       if ( ai->last_arch + ARCHIVE_EA_TEMPS_SI_VARIABLE < Partage->top ) { need_arch = TRUE; }

       switch ( ai->type )
        { case ENTREEANA_NON_INTERP:
               ai->val_ech = val_avant_ech;                                                        /* Pas d'interprétation !! */
               ai->inrange = 1;
               break;
          case ENTREEANA_4_20_MA_10BITS:
               if (val_avant_ech < 100)                                                /* 204) Modification du range pour 4mA */
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 204) val_avant_ech = 204;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-204)*(ai->max - ai->min))/820.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_4_20_MA_12BITS:
               if (val_avant_ech < 400)
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 816) val_avant_ech = 816;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-816)*(ai->max - ai->min))/3280.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_WAGO_750455:                                                                              /* 4/20 mA */
               ai->val_ech = (gfloat) (val_avant_ech*(ai->max - ai->min))/4095.0 + ai->min;
               ai->inrange = 1;
               break;
          case ENTREEANA_WAGO_750461:                                                                          /* Borne PT100 */
               if (val_avant_ech > -32767 && val_avant_ech < 8500)
                { ai->val_ech = (gfloat)(val_avant_ech/10.0);                                           /* Valeur à l'echelle */
                  ai->inrange = 1;
                }
               else ai->inrange = 0;
               break;
          default:
               ai->val_ech = 0.0;
               ai->inrange = 0;
        }
     }
    else if ( ai->last_arch + ARCHIVE_EA_TEMPS_SI_CONSTANT < Partage->top )
     { need_arch = TRUE; }                                                               /* Archive au pire toutes les 10 min */

    if (need_arch)
     { Ajouter_arch_by_nom( ai->acronyme, ai->tech_id, ai->val_ech );                              /* Archivage si besoin */
       ai->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_INT: Positionne un integer dans la mémoire DLS                                                                */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CI ( gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p, gboolean etat, gint reset, gint ratio )
  { struct DLS_CI *cpt_imp;

    if (!cpt_imp_p || !*cpt_imp_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_CI;
       while (liste)
        { cpt_imp = (struct DLS_CI *)liste->data;
          if ( !strcasecmp ( cpt_imp->acronyme, acronyme ) && !strcasecmp( cpt_imp->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { cpt_imp = g_try_malloc0 ( sizeof(struct DLS_CI) );
          if (!cpt_imp)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_imp->acronyme, sizeof(cpt_imp->acronyme), "%s", acronyme );
          g_snprintf( cpt_imp->tech_id,  sizeof(cpt_imp->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CI = g_slist_prepend ( Partage->Dls_data_CI, cpt_imp );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding CI '%s:%s'", __func__, tech_id, acronyme );
          Charger_conf_CI ( cpt_imp );                                     /* Chargement des valeurs en base pour ce compteur */
        }
       if (cpt_imp_p) *cpt_imp_p = (gpointer)cpt_imp;                   /* Sauvegarde pour acceleration si besoin */
      }
    else cpt_imp = (struct DLS_CI *)*cpt_imp_p;

    gboolean need_arch = FALSE;
    if (etat)
     { if (reset)                                                                       /* Le compteur doit-il etre resetté ? */
        { if (cpt_imp->valeur!=0)
           { cpt_imp->val_en_cours1 = 0;                                          /* Valeur transitoire pour gérer les ratio */
             cpt_imp->valeur = 0;                                                 /* Valeur transitoire pour gérer les ratio */
             need_arch = TRUE;
           }
        }
       else if ( cpt_imp->etat == FALSE )                                                             /* Passage en actif */
        { cpt_imp->etat = TRUE;
          cpt_imp->val_en_cours1++;
          if (cpt_imp->val_en_cours1>=ratio)
           { cpt_imp->valeur++;
             cpt_imp->val_en_cours1=0;                                                    /* RAZ de la valeur de calcul 1 */
             need_arch = TRUE;
           }
        }
     }
    else
     { if (reset==0) cpt_imp->etat = FALSE; }

    if (need_arch == TRUE)
     { Ajouter_arch_by_nom( cpt_imp->acronyme, cpt_imp->tech_id, cpt_imp->valeur*1.0 ); }  /* Archivage si besoin */
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur de l'EA en parametre                                                             */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CI ( gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p )
  { struct DLS_CI *cpt_imp;
    GSList *liste;
    if (cpt_imp_p && *cpt_imp_p)                                                     /* Si pointeur d'acceleration disponible */
     { cpt_imp = (struct DLS_CI *)*cpt_imp_p;
       return( cpt_imp->valeur );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_CI;
    while (liste)
     { cpt_imp = (struct DLS_CI *)liste->data;
       if ( !strcasecmp ( cpt_imp->acronyme, acronyme ) && !strcasecmp( cpt_imp->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0);
    if (cpt_imp_p) *cpt_imp_p = (gpointer)cpt_imp;                                  /* Sauvegarde pour acceleration si besoin */
    return( cpt_imp->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_set_INT: Positionne un integer dans la mémoire DLS                                                                */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CH ( gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p, gboolean etat, gint reset )
  { struct DLS_CH *cpt_h;

    if (!cpt_h_p || !*cpt_h_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_CH;
       while (liste)
        { cpt_h = (struct DLS_CH *)liste->data;
          if ( !strcasecmp ( cpt_h->acronyme, acronyme ) && !strcasecmp( cpt_h->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { cpt_h = g_try_malloc0 ( sizeof(struct DLS_CH) );
          if (!cpt_h)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_h->acronyme, sizeof(cpt_h->acronyme), "%s", acronyme );
          g_snprintf( cpt_h->tech_id,  sizeof(cpt_h->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CH = g_slist_prepend ( Partage->Dls_data_CH, cpt_h );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding CH '%s:%s'", __func__, tech_id, acronyme );
          Charger_conf_CH ( cpt_h );                                       /* Chargement des valeurs en base pour ce compteur */
        }
       if (cpt_h_p) *cpt_h_p = (gpointer)cpt_h;                                     /* Sauvegarde pour acceleration si besoin */
      }
    else cpt_h = (struct DLS_CH *)*cpt_h_p;

    if (reset)
     { if (etat)
        { cpt_h->valeur = 0;
          cpt_h->etat = FALSE;
        }
     }
    else if (etat)
     { if ( ! cpt_h->etat )
        { cpt_h->etat = TRUE;
          cpt_h->old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - cpt_h->old_top;
          if (delta > 600)                                           /* On compte +1 toutes les minutes ! */
           { cpt_h->valeur++;
             cpt_h->old_top = new_top;
             Ajouter_arch_by_nom( cpt_h->acronyme, cpt_h->tech_id, 1.0*cpt_h->valeur );
           }
        }
     }
    else
     { cpt_h->etat = FALSE; }
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur de l'EA en parametre                                                             */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CH ( gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p )
  { struct DLS_CH *cpt_h;
    GSList *liste;
    if (cpt_h_p && *cpt_h_p)                                                         /* Si pointeur d'acceleration disponible */
     { cpt_h = (struct DLS_CH *)*cpt_h_p;
       return( cpt_h->valeur );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_CH;
    while (liste)
     { cpt_h = (struct DLS_CH *)liste->data;
       if ( !strcasecmp ( cpt_h->acronyme, acronyme ) && !strcasecmp( cpt_h->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0);
    if (cpt_h_p) *cpt_h_p = (gpointer)cpt_h;                                        /* Sauvegarde pour acceleration si besoin */
    return( cpt_h->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gfloat Dls_data_get_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    GSList *liste;
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai->val_ech );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_AI;
    while (liste)
     { ai = (struct DLS_AI *)liste->data;
       if ( !strcasecmp ( ai->acronyme, acronyme ) && !strcasecmp( ai->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0.0);
    if (ai_p) *ai_p = (gpointer)ai;                                                 /* Sauvegarde pour acceleration si besoin */
    return( ai->val_ech );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_AI_inrange ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    Dls_data_get_AI ( tech_id, acronyme, ai_p );
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai->inrange );
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Dls_data_set_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_tempo ( gchar *tech_id, gchar *acronyme, gpointer *tempo_p, gboolean etat,
                           gint delai_on, gint min_on, gint max_on, gint delai_off, gint random)
  { struct DLS_TEMPO *tempo;

    if (!tempo_p || !*tempo_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_TEMPO;
       while (liste)
        { tempo = (struct DLS_TEMPO *)liste->data;
          if ( !strcasecmp ( tempo->acronyme, acronyme ) && !strcasecmp( tempo->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { tempo = g_try_malloc0 ( sizeof(struct DLS_TEMPO) );
          if (!tempo)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( tempo->acronyme, sizeof(tempo->acronyme), "%s", acronyme );
          g_snprintf( tempo->tech_id,  sizeof(tempo->tech_id),  "%s", tech_id );
          tempo->init = FALSE;
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_TEMPO = g_slist_prepend ( Partage->Dls_data_TEMPO, tempo );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding TEMPO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (tempo_p) *tempo_p = (gpointer)tempo;                                     /* Sauvegarde pour acceleration si besoin */
      }
    else tempo = (struct DLS_TEMPO *)*tempo_p;

    if (tempo->init == FALSE)
     { tempo->delai_on  = delai_on;
       tempo->min_on    = min_on;
       tempo->max_on    = max_on;
       tempo->delai_off = delai_off;
       tempo->random    = random;
       tempo->init      = TRUE;
     }
    ST_local ( tempo, etat );                                                                     /* Recopie dans la variable */
  }
/******************************************************************************************************************************/
/* Dls_data_get_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_tempo ( gchar *tech_id, gchar *acronyme, gpointer *tempo_p )
  { struct DLS_TEMPO *tempo;
    GSList *liste;
    if (tempo_p && *tempo_p)                                                         /* Si pointeur d'acceleration disponible */
     { tempo = (struct DLS_TEMPO *)*tempo_p;
       return( tempo->state );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_TEMPO;
    while (liste)
     { tempo = (struct DLS_TEMPO *)liste->data;
       if ( !strcasecmp ( tempo->acronyme, acronyme ) && !strcasecmp( tempo->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (tempo_p) *tempo_p = (gpointer)tempo;                                        /* Sauvegarde pour acceleration si besoin */
    return( tempo->state );
  }
/******************************************************************************************************************************/
/* Dls_data_set_bus : Envoi un message sur le bus système                                                                     */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et les paramètres du message                                  */
/******************************************************************************************************************************/
 void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p, gboolean etat,
                         gchar *host, gchar *thread, gchar *tag, gchar *param1)
  { Dls_data_set_bool ( tech_id, acronyme, bus_p, etat );                                         /* Utilisation d'un boolean */
    if (Dls_data_get_bool_up(tech_id, acronyme, bus_p))
     { if (param1)
        { Send_zmq_with_tag ( Partage->com_dls.zmq_to_master, NULL, "dls", host, thread, tag, param1, strlen(param1)+1 ); }
       else
        { Send_zmq_with_tag ( Partage->com_dls.zmq_to_master, NULL, "dls", host, thread, tag, NULL, 0 ); }
     }
    if (param1) g_free(param1);                                       /* Param1 est issu d'un g_strdup ou d'un Dls_dyn_string */
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MSG ( gchar *tech_id, gchar *acronyme, gpointer *msg_p, gboolean etat )
  { struct DLS_MESSAGES *msg;

    if (!msg_p || !*msg_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_MSG;
       while (liste)
        { msg = (struct DLS_MESSAGES *)liste->data;
          if ( !strcasecmp ( msg->acronyme, acronyme ) && !strcasecmp( msg->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { msg = g_try_malloc0 ( sizeof(struct DLS_MESSAGES) );
          if (!msg)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( msg->acronyme, sizeof(msg->acronyme), "%s", acronyme );
          g_snprintf( msg->tech_id,  sizeof(msg->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_MSG = g_slist_prepend ( Partage->Dls_data_MSG, msg );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding MSG '%s:%s'", __func__, tech_id, acronyme );
        }
       if (msg_p) *msg_p = (gpointer)msg;                                           /* Sauvegarde pour acceleration si besoin */
      }
    else msg = (struct DLS_MESSAGES *)*msg_p;

    if ( msg->etat != etat )
     { msg->etat = etat;

       if ( msg->last_change + 10 <= Partage->top ) { msg->changes = 0; }            /* Si pas de change depuis plus de 1 sec */

       if ( msg->changes > 5 && !(Partage->top % 50) )              /* Si persistence d'anomalie on prévient toutes les 5 sec */
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: last_change trop tot for MSG '%s':'%s' !", __func__,
                    msg->tech_id, msg->acronyme );
        }
       else
        { struct MESSAGES_EVENT *event;
          event = (struct MESSAGES_EVENT *)g_try_malloc0( sizeof ( struct MESSAGES_EVENT ) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->num  = -1;
             event->etat = etat;                                                        /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
             Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : Changing DLS_MSG '%s:%s'=%d",
                       __func__, msg->tech_id, msg->acronyme, etat );
           }
          msg->changes++;
          msg->last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MSG ( gchar *tech_id, gchar *acronyme, gpointer *msg_p )
  { struct DLS_MESSAGES *msg;
    GSList *liste;
    if (msg_p && *msg_p)                                                             /* Si pointeur d'acceleration disponible */
     { msg = (struct DLS_MESSAGES *)*msg_p;
       return( msg->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_MSG;
    while (liste)
     { msg = (struct DLS_MESSAGES *)liste->data;
       if ( !strcasecmp ( msg->acronyme, acronyme ) && !strcasecmp( msg->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (msg_p) *msg_p = (gpointer)msg;                                              /* Sauvegarde pour acceleration si besoin */
    return( msg->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_set_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL ( gchar *tech_id, gchar *acronyme, gpointer *visu_p, gint mode,
                            gchar *color, gboolean cligno )
  { struct DLS_VISUEL *visu;

    if (!visu_p || !*visu_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_VISUEL;
       while (liste)
        { visu = (struct DLS_VISUEL *)liste->data;
          if ( !strcasecmp ( visu->acronyme, acronyme ) && !strcasecmp( visu->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { visu = g_try_malloc0 ( sizeof(struct DLS_VISUEL) );
          if (!visu)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( visu->acronyme, sizeof(visu->acronyme), "%s", acronyme );
          g_snprintf( visu->tech_id,  sizeof(visu->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_VISUEL = g_slist_prepend ( Partage->Dls_data_VISUEL, visu );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : adding VISUEL '%s:%s'", __func__, tech_id, acronyme );
        }
       if (visu_p) *visu_p = (gpointer)visu;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else visu = (struct DLS_VISUEL *)*visu_p;

    if (visu->mode != mode || strcmp( visu->color, color ) || visu->cligno != cligno )
     { if ( visu->last_change + 50 <= Partage->top )                                 /* Si pas de change depuis plus de 5 sec */
        { visu->changes = 0; }

       if ( visu->changes <= 10 )                                                          /* Si moins de 10 changes en 5 sec */
        { if ( visu->changes == 10 )                                                /* Est-ce le dernier change avant blocage */
           { visu->mode   = 0;                                                   /* Si oui, on passe le visuel en kaki cligno */
             g_snprintf( visu->color, sizeof(visu->color), "brown" );
             visu->cligno = 1;                                                                                  /* Clignotant */
           }
          else { visu->mode   = mode;                                /* Sinon on recopie ce qui est demandé par le plugin DLS */
                 g_snprintf( visu->color, sizeof(visu->color), "%s", color );
                 visu->cligno = cligno;
               }

          visu->last_change = Partage->top;                                                             /* Date de la photo ! */
          pthread_mutex_lock( &Partage->com_msrv.synchro );                             /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_new_i = g_slist_append( Partage->com_msrv.liste_new_i, visu );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
        }
       visu->changes++;                                                                                /* Un change de plus ! */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_VISUEL ( gchar *tech_id, gchar *acronyme, gpointer *visu_p )
  { struct DLS_VISUEL *visu;
    GSList *liste;
    if (visu_p && *visu_p)                                                             /* Si pointeur d'acceleration disponible */
     { visu = (struct DLS_VISUEL *)*visu_p;
       return( visu->mode );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_VISUEL;
    while (liste)
     { visu = (struct DLS_VISUEL *)liste->data;
       if ( !strcasecmp ( visu->acronyme, acronyme ) && !strcasecmp( visu->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (visu_p) *visu_p = (gpointer)visu;                                           /* Sauvegarde pour acceleration si besoin */
    return( visu->mode );
  }
/******************************************************************************************************************************/
/* Dls_dyn_string: Formate la chaine en parametre avec le bit également en parametre                                          */
/* Entrée : La chaine source, le type de bit, le tech_id/acronyme, le pointeur de raccourci                                   */
/* sortie : Une nouvelle chaine de caractere à g_freer                                                                        */
/******************************************************************************************************************************/
 gchar *Dls_dyn_string ( gchar *format, gint type_bit, gchar *tech_id, gchar *acronyme, gpointer *dlsdata_p )
  { gchar result[128], *debut, chaine[64];
    struct DB *db;
    debut = g_strrstr ( format, "$1" );                            /* Début pointe sur le $ de "$1" si présent dans la chaine */
    if (!debut) return(g_strdup(format));
    g_snprintf( result, debut-format+1, "%s", format );                                                           /* Prologue */
    switch (type_bit)
     { case MNEMO_CPT_IMP:
            if ( (db=Rechercher_CI ( tech_id, acronyme )) != NULL )
             { gint valeur = Dls_data_get_CI ( tech_id, acronyme, dlsdata_p );
               g_snprintf( chaine, sizeof(chaine), "%d %s", valeur, db->row[1] ); /* Row1 = unite */
               Libere_DB_SQL (&db);
             }
            break;
       case MNEMO_ENTREE_ANA:
            if (!strcasecmp(tech_id, "SYS") && !strcasecmp(acronyme, "TIME"))
             { struct tm tm;
               time_t temps;
               time(&temps);
               localtime_r( &temps, &tm );
               g_snprintf( chaine, sizeof(chaine), "%d heure et %d minute", tm.tm_hour, tm.tm_min );
             }
            else
             { Dls_data_get_AI ( tech_id, acronyme, dlsdata_p );
               struct DLS_AI *ai = *dlsdata_p;
               if (ai)
                { if (ai->val_ech-roundf(ai->val_ech) == 0.0)
                   { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
                  else
                   { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->val_ech, ai->unite ); }
                }
               else g_snprintf( chaine, sizeof(chaine), "erreur" );
             }
            break;
       default: return(NULL);
     }
    g_strlcat ( result, chaine, sizeof(result) );
    g_strlcat ( result, debut+2, sizeof(result) );
    return(g_strdup(result));
  }
/******************************************************************************************************************************/
/* Dls_foreach_dls_tree: Parcours recursivement l'arbre DLS et execute des commandes en parametres                            */
/* Entrée : le Dls_tree et les fonctions a appliquer                                                                          */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_foreach_dls_tree ( struct DLS_TREE *dls_tree, void *user_data,
                                    void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                                    void (*do_tree)   (void *user_data, struct DLS_TREE *) )
  { GSList *liste;
    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_tree;
       sub_tree = (struct DLS_TREE *)liste->data;
       Dls_foreach_dls_tree( sub_tree, user_data, do_plugin, do_tree );
       liste = liste->next;
     }
    liste = dls_tree->Liste_plugin_dls;
    while(liste && do_plugin)                                                        /* On execute tous les modules un par un */
     { struct PLUGIN_DLS *plugin_actuel;
       plugin_actuel = (struct PLUGIN_DLS *)liste->data;
       do_plugin( user_data, plugin_actuel );
       liste = liste->next;
     }
    if (do_tree) do_tree( user_data, dls_tree );
  }
/******************************************************************************************************************************/
/* Dls_foreach: Parcours l'arbre DLS et execute des commandes en parametres                                                   */
/* Entrée : les fonctions a appliquer                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_foreach ( void *user_data, void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                                     void (*do_tree)   (void *user_data, struct DLS_TREE *) )
  { if (Partage->com_dls.Dls_tree)
     { pthread_mutex_lock( &Partage->com_dls.synchro );
       Dls_foreach_dls_tree( Partage->com_dls.Dls_tree, user_data, do_plugin, do_tree );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
     }
  }
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_dls_tree ( struct DLS_TREE *dls_tree )
  { struct timeval tv_avant, tv_apres;
    gboolean bit_comm_out, bit_defaut, bit_defaut_fixe, bit_alarme, bit_alarme_fixe;                              /* Activité */
    gboolean bit_veille_partielle, bit_veille_totale, bit_alerte, bit_alerte_fixe;             /* Synthese Sécurité des Biens */
    gboolean bit_derangement, bit_derangement_fixe, bit_danger, bit_danger_fixe;           /* synthèse Sécurité des Personnes */
    GSList *liste;

    bit_comm_out = bit_defaut = bit_defaut_fixe = bit_alarme = bit_alarme_fixe = FALSE;
    bit_veille_partielle = FALSE;
    bit_veille_totale = TRUE;
    bit_alerte = bit_alerte_fixe = FALSE;
    bit_derangement = bit_derangement_fixe = bit_danger = bit_danger_fixe = FALSE;

    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                     /* On execute tous les modules un par un */
     { struct PLUGIN_DLS *plugin_actuel;
       plugin_actuel = (struct PLUGIN_DLS *)liste->data;

       if (plugin_actuel->plugindb.on && plugin_actuel->go)
        { gettimeofday( &tv_avant, NULL );
          Partage->top_cdg_plugin_dls = 0;                                                      /* On reset le cdg plugin DLS */
          plugin_actuel->go( &plugin_actuel->vars );     /* On appel le plugin */
          gettimeofday( &tv_apres, NULL );
          plugin_actuel->conso+=Chrono( &tv_avant, &tv_apres );
          plugin_actuel->vars.starting = 0;

          plugin_actuel->vars.bit_acquit = 0;                                                 /* On arrete l'acquit du plugin */
                                                                                                  /* Bit de synthese activite */
          bit_comm_out         |= plugin_actuel->vars.bit_comm_out;
          bit_defaut           |= plugin_actuel->vars.bit_defaut;
          bit_defaut_fixe      |= plugin_actuel->vars.bit_defaut_fixe;
          bit_alarme           |= plugin_actuel->vars.bit_alarme;
          bit_alarme_fixe      |= plugin_actuel->vars.bit_alarme_fixe;
          plugin_actuel->vars.bit_activite_ok = !(bit_comm_out | bit_defaut | bit_defaut_fixe | bit_alarme | bit_alarme_fixe);

          bit_veille_partielle |= plugin_actuel->vars.bit_veille;
          bit_veille_totale    &= plugin_actuel->vars.bit_veille;
          bit_alerte           |= plugin_actuel->vars.bit_alerte;
          bit_alerte_fixe      |= plugin_actuel->vars.bit_alerte_fixe;

          bit_derangement      |= plugin_actuel->vars.bit_derangement;
          bit_derangement_fixe |= plugin_actuel->vars.bit_derangement_fixe;
          bit_danger           |= plugin_actuel->vars.bit_danger;
          bit_danger_fixe      |= plugin_actuel->vars.bit_danger_fixe;
          plugin_actuel->vars.bit_secupers_ok = !(bit_derangement | bit_derangement_fixe | bit_danger | bit_danger_fixe);
        }
       liste = liste->next;
     }
    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_tree;
       sub_tree = (struct DLS_TREE *)liste->data;
       Dls_run_dls_tree ( sub_tree );
       bit_comm_out         |= sub_tree->syn_vars.bit_comm_out;
       bit_defaut           |= sub_tree->syn_vars.bit_defaut;
       bit_defaut_fixe      |= sub_tree->syn_vars.bit_defaut_fixe;
       bit_alarme           |= sub_tree->syn_vars.bit_alarme;
       bit_alarme_fixe      |= sub_tree->syn_vars.bit_alarme_fixe;
       bit_veille_partielle |= sub_tree->syn_vars.bit_veille_partielle;
       bit_veille_totale    &= sub_tree->syn_vars.bit_veille_totale;
       bit_alerte           |= sub_tree->syn_vars.bit_alerte;
       bit_alerte_fixe      |= sub_tree->syn_vars.bit_alerte_fixe;
       bit_derangement      |= sub_tree->syn_vars.bit_derangement;
       bit_derangement_fixe |= sub_tree->syn_vars.bit_derangement_fixe;
       bit_danger           |= sub_tree->syn_vars.bit_danger;
       bit_danger_fixe      |= sub_tree->syn_vars.bit_danger_fixe;
       liste = liste->next;
     }

    if ( bit_comm_out         != dls_tree->syn_vars.bit_comm_out ||                              /* Detection des changements */
         bit_defaut           != dls_tree->syn_vars.bit_defaut ||
         bit_defaut_fixe      != dls_tree->syn_vars.bit_defaut_fixe ||
         bit_alarme           != dls_tree->syn_vars.bit_alarme ||
         bit_alarme_fixe      != dls_tree->syn_vars.bit_alarme_fixe ||
         bit_veille_partielle != dls_tree->syn_vars.bit_veille_partielle ||
         bit_veille_totale    != dls_tree->syn_vars.bit_veille_totale ||
         bit_alerte           != dls_tree->syn_vars.bit_alerte ||
         bit_alerte_fixe      != dls_tree->syn_vars.bit_alerte_fixe ||
         bit_derangement      != dls_tree->syn_vars.bit_derangement ||
         bit_derangement_fixe != dls_tree->syn_vars.bit_derangement_fixe ||
         bit_danger           != dls_tree->syn_vars.bit_danger ||
         bit_danger_fixe      != dls_tree->syn_vars.bit_danger_fixe )
     { dls_tree->syn_vars.bit_comm_out         = bit_comm_out;                           /* Recopie et envoi aux threads SSRV */
       dls_tree->syn_vars.bit_defaut           = bit_defaut;
       dls_tree->syn_vars.bit_defaut_fixe      = bit_defaut_fixe;
       dls_tree->syn_vars.bit_alarme           = bit_alarme;
       dls_tree->syn_vars.bit_alarme_fixe      = bit_alarme_fixe;
       dls_tree->syn_vars.bit_veille_partielle = bit_veille_partielle;
       dls_tree->syn_vars.bit_veille_totale    = bit_veille_totale;
       dls_tree->syn_vars.bit_alerte           = bit_alerte;
       dls_tree->syn_vars.bit_alerte_fixe      = bit_alerte_fixe;
       dls_tree->syn_vars.bit_derangement      = bit_derangement;
       dls_tree->syn_vars.bit_derangement_fixe = bit_derangement_fixe;
       dls_tree->syn_vars.bit_danger           = bit_danger;
       dls_tree->syn_vars.bit_danger_fixe      = bit_danger_fixe;
       Send_zmq_with_tag ( Partage->com_dls.zmq_to_master,
                           NULL, "dls", "*", "ssrv", "SET_SYN_VARS",
                          &dls_tree->syn_vars, sizeof(struct CMD_TYPE_SYN_VARS) );
     }
 }
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint Update_heure=0;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Partage->com_dls.Thread_run         = TRUE;                                                         /* Le thread tourne ! */
    Dls_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */
    Charger_plugins();                                                                          /* Chargement des modules dls */
    SB_SYS(1, 0);                                                                                      /* B1 est toujours à 0 */
    SB_SYS(2, 1);                                                                                      /* B2 est toujours à 1 */
    sleep(30);                    /* attente 30 secondes pour initialisation des bit internes et collection des infos modules */

    Partage->com_dls.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     {

       if (Partage->com_dls.Thread_reload)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: RELOADING", __func__ );
          Dls_Lire_config();
          Decharger_plugins();
          Charger_plugins();
          Partage->com_dls.Thread_reload = FALSE;
        }

       if (Partage->top-Update_heure>=600)                          /* Gestion des changements d'horaire (toutes les minutes) */
        { Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
          Update_heure=Partage->top;
        }

       if (Partage->com_dls.admin_start)                                                      /* A-t-on un plugin a allumer ? */
        { Activer_plugin_by_id ( Partage->com_dls.admin_start, TRUE );
          Partage->com_dls.admin_start = 0;
        }

       if (Partage->com_dls.admin_stop)                                                      /* A-t-on un plugin a eteindre ? */
        { Activer_plugin_by_id ( Partage->com_dls.admin_stop, FALSE );
          Partage->com_dls.admin_stop = 0;
        }

       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

       SB_SYS(0, !B(0));                                                            /* Change d'etat tous les tours programme */
       SI(1, 1, 255, 0, 0, 0 );                                                                   /* Icone toujours à 1:rouge */

       pthread_mutex_lock( &Partage->com_dls.synchro );
       Dls_run_dls_tree( Partage->com_dls.Dls_tree );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
       SB_SYS(3, 1);                                                  /* B3 est toujours à un apres le premier tour programme */
       Partage->com_dls.Top_check_horaire = FALSE;                         /* Cotrole horaire effectué un fois par minute max */
       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Decharger_plugins();                                                                      /* Dechargement des modules DLS */
    Close_zmq(Partage->com_dls.zmq_to_master);
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: DLS Down (%p)", __func__, pthread_self() );
    Partage->com_dls.TID = 0;                                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
