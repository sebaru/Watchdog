/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/ligne.y        Définitions des ligne dls DLS                                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        jeu. 24 juin 2010 19:37:44 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * lignes.y
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 
%{
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "Proto_traductionDLS.h"
#include "Dls.h"
%}

%union { int val;
         float valf;
         char *chaine;
         GList *gliste;
         struct OPTION *option;
         struct ACTION *action;
         struct COMPARATEUR *comparateur;
       };

%token <val>    T_ERROR PVIRGULE VIRGULE T_DPOINTS DONNE EQUIV MOINS T_POUV T_PFERM T_EGAL OU ET BARRE T_FOIS T_DEFINE T_STATIC

%token <val>    T_SBIEN_VEILLE T_SBIEN_ALE T_SBIEN_ALEF T_TOP_ALERTE
%token <val>    T_SPERS_DER T_SPERS_DERF T_SPERS_DAN T_SPERS_DANF T_OSYN_ACQ
%token <val>    T_ACT_COMOUT T_ACT_DEF T_ACT_ALA T_ACT_DEFF T_ACT_ALAF  T_ACT_DOWN
%token <val>    T_BUS T_HOST T_THREAD T_TAG T_PARAM1

%token <val>    MODE CONSIGNE COLOR CLIGNO RESET RATIO T_LIBELLE T_ETIQUETTE
%token <val>    T_DAA T_DMINA T_DMAXA T_DAD T_RANDOM

%token <val>    T_TYPE T_ETAT T_ATTENTE T_DEFAUT T_ALARME T_VEILLE T_ALERTE T_DERANGEMENT T_DANGER
%type  <val>    type_msg

%token <val>    INF SUP INF_OU_EGAL SUP_OU_EGAL T_TRUE T_FALSE
%type  <val>    ordre

%token <val>    HEURE APRES AVANT LUNDI MARDI MERCREDI JEUDI VENDREDI SAMEDI DIMANCHE
%type  <val>    modulateur jour_semaine

%token <val>    T_BI T_MONO ENTREE SORTIE T_TEMPO T_HORLOGE
%token <val>    T_MSG ICONE CPT_H T_CPT_IMP EANA T_START T_REGISTRE
%type  <val>    alias_bit

%token <val>    ROUGE VERT BLEU JAUNE NOIR BLANC ORANGE GRIS KAKI T_EDGE_UP
%type  <val>    couleur

%token <chaine> ID T_CHAINE
%token <val>    ENTIER
%token <valf>   VALF

%type  <val>         barre calcul_ea_result
%type  <gliste>      liste_options options
%type  <option>      une_option
%type  <chaine>      unite facteur expr suffixe
%type  <action>      action une_action
%type  <comparateur> comparateur
%type  <chaine>      calcul_expr calcul_expr2 calcul_expr3

%%
fichier: ligne_source_dls;

ligne_source_dls:         listeAlias listeInstr
                        | listeAlias
                        | listeInstr
                        |
                        ;

/*************************************************** Gestion des alias ********************************************************/
listeAlias:     un_alias listeAlias
                | un_alias
                ;
                
un_alias:       T_DEFINE ID EQUIV alias_bit liste_options PVIRGULE
                {{ int taille;
                   if ( New_alias(ALIAS_TYPE_DYNAMIC, NULL, $2, $4, -1, 0, $5) == FALSE )                    /* Deja defini ? */
                    { Emettre_erreur_new( "Ligne %d: '%s' is already defined", DlsScanner_get_lineno(), $2 ); }
                   g_free($2);
                }}
                | T_STATIC ID EQUIV barre alias_bit ENTIER PVIRGULE
                {{ int taille;
                   switch($5)
                    { case MNEMO_ENTREE:
                      case MNEMO_SORTIE:
                      case MNEMO_BISTABLE:
                                 if ( New_alias(ALIAS_TYPE_STATIC, NULL, $2, $5, $6, $4, NULL) == FALSE )    /* Deja defini ? */
                                  { Emettre_erreur_new( "Ligne %d: '%s' is already defined", DlsScanner_get_lineno(), $2 ); }
                                 break;
                      case MNEMO_TEMPO:
                                 Emettre_erreur_new( "Ligne %d: Use of #static _T is obsolete", DlsScanner_get_lineno() );
                                 break;
                      case MNEMO_ENTREE_ANA:
                      case MNEMO_MONOSTABLE  :
                      case MNEMO_CPTH:
                      case MNEMO_CPT_IMP:
                      case MNEMO_MSG:
                      case MNEMO_REGISTRE :
                      case MNEMO_MOTIF:
                                 if ($4==1)                                                                   /* Barre = 1 ?? */
                                  { Emettre_erreur_new( "Ligne %d: Use of '/%s' is forbidden", DlsScanner_get_lineno(), $2 ); }
                                 else
                                  { if (New_alias(ALIAS_TYPE_STATIC, NULL, $2, $5, $6, 0, NULL) == FALSE)
                                     { Emettre_erreur_new( "Ligne %d: '%s' is already defined", DlsScanner_get_lineno(), $2 ); }
                                  }
                                 break;
                      default: Emettre_erreur_new( "Ligne %d: Syntaxe Error near '%s'", DlsScanner_get_lineno(), $2 );
                               break;
                    }
                   g_free($2);
                }}
                ;
alias_bit:        T_BI       {{ $$=MNEMO_BISTABLE;   }}
                | T_MONO     {{ $$=MNEMO_MONOSTABLE; }}
                | ENTREE     {{ $$=MNEMO_ENTREE;     }}
                | SORTIE     {{ $$=MNEMO_SORTIE;     }}
                | T_MSG      {{ $$=MNEMO_MSG;        }}
                | T_TEMPO    {{ $$=MNEMO_TEMPO;      }}
                | ICONE      {{ $$=MNEMO_MOTIF;      }}
                | CPT_H      {{ $$=MNEMO_CPTH;       }}
                | T_CPT_IMP  {{ $$=MNEMO_CPT_IMP;    }}
                | EANA       {{ $$=MNEMO_ENTREE_ANA; }}
                | T_REGISTRE {{ $$=MNEMO_REGISTRE;   }}
                | T_HORLOGE  {{ $$=MNEMO_HORLOGE;    }}
                | T_BUS      {{ $$=MNEMO_BUS;        }}
                ;
/**************************************************** Gestion des instructions ************************************************/
listeInstr:     une_instr listeInstr
                | une_instr
                ;

une_instr:      MOINS expr DONNE action PVIRGULE
                {{ int taille;
                   char *instr;
                   taille = strlen($2)+strlen($4->alors)+15;
                   if ($4->sinon)
                    { taille += (strlen($4->sinon) + 10);
                      instr = New_chaine( taille );
                      g_snprintf( instr, taille, "if(%s)\n { %s }\nelse\n { %s }\n", $2, $4->alors, $4->sinon );
                    }
                   else
                    { instr = New_chaine( taille );
                      g_snprintf( instr, taille, "if(%s)\n { %s }\n", $2, $4->alors );
                    }

                   Emettre( instr ); g_free(instr);
                   if ($4->sinon) g_free($4->sinon); 
                   g_free($4->alors); g_free($4);
                   g_free($2);
                }}
                | MOINS expr MOINS T_POUV calcul_expr T_PFERM DONNE calcul_ea_result PVIRGULE
                {{ int taille;
                   char *instr;
                   taille = strlen($5)+strlen($2)+35;
                   instr = New_chaine( taille );
                   g_snprintf( instr, taille, "if(%s) { SR(%d,%s); }\n", $2, $8, $5 );
                   Emettre( instr ); g_free(instr);
                   g_free($2);
                   g_free($5);
                }}
                ;
/****************************************************** Partie CALCUL *********************************************************/
calcul_expr:    calcul_expr OU calcul_expr2
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s+%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr MOINS calcul_expr2
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s-%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr2
                ;
calcul_expr2:   calcul_expr2 T_FOIS calcul_expr3
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s*%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr2 BARRE calcul_expr3
                {{ int taille;
                   taille = strlen($1) + strlen($3) + 4;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s/%s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | calcul_expr3
                ;
calcul_expr3:   VALF
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "%f", $1 );
                }}
                | ENTIER
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "%d", $1 );
                }}
                | EANA ENTIER
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "EA_ech(%d)", $2 );
                }}
                | T_REGISTRE ENTIER
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "R(%d)", $2 );
                }}
                | T_CPT_IMP ENTIER
                {{ int taille;
                   taille = 15;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "CI(%d)", $2 );
                }}
                | T_POUV calcul_expr T_PFERM
                {{ $$=$2; }}
                | ID
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_acronyme(NULL,$1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_ENTREE_ANA:
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "EA_ech(%d)", alias->num );
                            break;
                          }
                         case MNEMO_REGISTRE:
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "R(%d)", alias->num );
                            break;
                          }
                         case MNEMO_CPT_IMP:
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "CI(%d)", alias->num );
                            break;
                          }
                         default: 
                          { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser dans un calcul", DlsScanner_get_lineno(), $1 );
                            $$=New_chaine(2);
                            g_snprintf( $$, 2, "0" );
                          }
                       }
                    }
                   else 
                    { Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), $1 );
                      $$=New_chaine(2);
                      g_snprintf( $$, 2, "0" );
                    }
                   g_free($1);                                     /* On n'a plus besoin de l'identifiant */
                }}
                ;

calcul_ea_result: T_REGISTRE ENTIER
                {{ $$ = $2;
                }}
                | ID
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_acronyme(NULL,$1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_REGISTRE:
                          { $$ = alias->num;
                            break;
                          }
                         default: 
                          { Emettre_erreur_new( "Ligne %d :'%s' ne peut s'utiliser dans un résultat de calcul", DlsScanner_get_lineno(), $1 );
                            $$=0;
                          }
                       }
                    }
                   else
                    { Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), $1 );
                      $$=0;
                    }
                   g_free($1);                                     /* On n'a plus besoin de l'identifiant */
                }}
                ;
/******************************************************* Partie LOGIQUE *******************************************************/
expr:           expr OU facteur
                {{ int taille;
                   taille = strlen($1)+strlen($3)+7;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s || %s)", $1, $3 );
                   g_free($1); g_free($3);
                }}

                | facteur
                ;
facteur:        facteur ET unite
                {{ int taille;
                   taille = strlen($1)+strlen($3)+7;
                   $$ = New_chaine( taille );
                   g_snprintf( $$, taille, "(%s && %s)", $1, $3 );
                   g_free($1); g_free($3);
                }}
                | unite
                ;

unite:          modulateur ENTIER HEURE ENTIER
                {{ int taille;
                   taille = 20;
                   $$ = New_chaine(taille);
                   if ($2>23) $2=23;
                   if ($4>59) $4=59;
                   switch ($1)
                    { case 0    : g_snprintf( $$, taille, "Heure(%d,%d)", $2, $4 );
                                  break;
                      case APRES: g_snprintf( $$, taille, "Heure_apres(%d,%d)", $2, $4 );
                                  break;
                      case AVANT: g_snprintf( $$, taille, "Heure_avant(%d,%d)", $2, $4 );
                                  break;
                    }
                }}
                | jour_semaine
                {{ int taille;
                   taille = 18;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "Jour_semaine(%d)", $1 );
                }}
                | T_START
                {{ int taille;
                   taille = 20;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(vars->starting)" );
                }}
                | T_TRUE
                {{ int taille;
                   taille = 5;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(1)" );
                }}
                | T_FALSE
                {{ int taille;
                   taille = 5;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(0)" );
                }}
                | T_TOP_ALERTE
                {{ int taille;
                   taille = 22;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "Dls_get_top_alerte()" );
                }}
                | T_OSYN_ACQ
                {{ $$ = g_strdup("vars->bit_acquit");
                }}
                | barre T_ACT_DOWN
                {{ if ($1) $$ = g_strdup("!vars->bit_activite_down");
                      else $$ = g_strdup("vars->bit_activite_down");
                }}
                | barre T_BI ENTIER
                {{ int taille;
                   taille = 10;
                   $$ = New_chaine( taille );
                   if ($1) { g_snprintf( $$, taille, "!B(%d)", $3 ); }
                   else    { g_snprintf( $$, taille, "B(%d)", $3 ); }
                }}
                | barre ENTREE ENTIER liste_options
                {{ $$ = New_condition_entree_old ( $1, $3, $4 );
                   Liberer_options($4);
                }}
               | EANA ENTIER ordre VALF
                {{ int taille;
                   if ($3 == T_EGAL)
                    { Emettre_erreur_new( "Ligne %d: EA%04d ne peut s'utiliser avec le comparateur '='", DlsScanner_get_lineno(), $2 );
                      $$=New_chaine(2);
                      g_snprintf( $$, 2, "0" ); 
                    }
                   else
                    { taille = 40;
                      $$ = New_chaine( taille );
                      switch( $3 )
                       { case INF        : g_snprintf( $$, taille, "EA_ech_inf(%f,%d)", $4, $2 ); break;
                         case SUP        : g_snprintf( $$, taille, "EA_ech_sup(%f,%d)", $4, $2 ); break;
                         case INF_OU_EGAL: g_snprintf( $$, taille, "EA_ech_inf_egal(%f,%d)", $4, $2 ); break;
                         case SUP_OU_EGAL: g_snprintf( $$, taille, "EA_ech_sup_egal(%f,%d)", $4, $2 ); break;
                       }
                    }
                }}
               | T_REGISTRE ENTIER ordre VALF
                {{ int taille;
                   taille = 40;
                   $$ = New_chaine( taille );
                   switch( $3 )
                    { case INF        : g_snprintf( $$, taille, "R(%d)<%f", $2, $4 ); break;
                      case SUP        : g_snprintf( $$, taille, "R(%d)>%f", $2, $4 ); break;
                      case INF_OU_EGAL: g_snprintf( $$, taille, "R(%d)<=%f", $2, $4 ); break;
                      case SUP_OU_EGAL: g_snprintf( $$, taille, "R(%d)>=%f", $2, $4 ); break;
                      case T_EGAL     : g_snprintf( $$, taille, "R(%d)==%f", $2, $4 ); break;
                    }
                }}
                | T_CPT_IMP ordre VALF
                {{ int taille;
                   taille = 30;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   switch( $1 )
                    { case INF        : g_snprintf( $$, taille, "CI(%d)<%f", $1, $3 );  break;
                      case SUP        : g_snprintf( $$, taille, "CI(%d)>%f", $1, $3 );  break;
                      case INF_OU_EGAL: g_snprintf( $$, taille, "CI(%d)<=%f", $1, $3 ); break;
                      case SUP_OU_EGAL: g_snprintf( $$, taille, "CI(%d)>=%f", $1, $3 ); break;
                      case T_EGAL     : g_snprintf( $$, taille, "CI(%d)==%f", $1, $3 ); break;
                    }
                }}
                | barre T_ACT_COMOUT
                  {{ $$=New_condition_vars( $1, "vars->bit_comm_out"); }}
                | barre T_ACT_DEF
                  {{ $$=New_condition_vars( $1, "vars->bit_defaut"); }}
                | barre T_ACT_DEFF
                  {{ $$=New_condition_vars( $1, "vars->bit_defaut_fixe"); }}
                | barre T_ACT_ALA
                  {{ $$=New_condition_vars( $1, "vars->bit_alarme"); }}
                | barre T_ACT_ALAF
                  {{ $$=New_condition_vars( $1, "vars->bit_alarme_fixe"); }}
                | barre T_SBIEN_VEILLE
                  {{ $$=New_condition_vars( $1, "vars->bit_veille"); }}
                | barre T_SBIEN_ALE
                  {{ $$=New_condition_vars( $1, "vars->bit_alerte"); }}
                | barre T_SBIEN_ALEF
                  {{ $$=New_condition_vars( $1, "vars->bit_alerte_fixe"); }}
                | barre T_SPERS_DER
                  {{ $$=New_condition_vars( $1, "vars->bit_derangement"); }}
                | barre T_SPERS_DERF
                  {{ $$=New_condition_vars( $1, "vars->bit_derangement_fixe"); }}
                | barre T_SPERS_DAN
                  {{ $$=New_condition_vars( $1, "vars->bit_danger"); }}
                | barre T_SPERS_DANF
                  {{ $$=New_condition_vars( $1, "vars->bit_danger_fixe"); }}
                | barre T_POUV expr T_PFERM
                {{ int taille;
                   taille = strlen($3)+5;
                   $$ = New_chaine( taille );
                   if ($1) { g_snprintf( $$, taille, "!(%s)", $3 ); }
                   else    { g_snprintf( $$, taille, "(%s)", $3 ); }
                   g_free($3);
                }}
                | barre ID suffixe liste_options comparateur
                {{ struct ALIAS *alias;
                   char *chaine, *tech_id, *acro;
                   int taille;
                   if ($3) { tech_id = $2; acro = $3; }
                      else { tech_id = NULL; acro = $2; }
                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias && $3) { alias = Set_new_external_alias(tech_id,acro); }/* Si dependance externe, on va chercher */
                   if (alias)
                    { if ($5 && (alias->bit==MNEMO_TEMPO ||                              /* Vérification des bits non comparables */
                                 alias->bit==MNEMO_ENTREE ||
                                 alias->bit==MNEMO_BISTABLE ||
                                 alias->bit==MNEMO_MONOSTABLE ||
                                 alias->bit==MNEMO_HORLOGE)
                         )
                       { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser dans une comparaison", DlsScanner_get_lineno(), $3 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );                                      
                       } else
                      if (!$5 && (alias->bit==MNEMO_ENTREE_ANA ||        /* Vérification des bits obligatoirement comparables */
                                  alias->bit==MNEMO_REGISTRE ||
                                  alias->bit==MNEMO_CPT_IMP ||
                                  alias->bit==MNEMO_CPTH)
                         )
                       { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser qu'avec une comparaison", DlsScanner_get_lineno(), $3 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );                                      
                       } 
                      else switch(alias->bit)                              /* On traite que ce qui peut passer en "condition" */
                       { case MNEMO_TEMPO :
                          { $$ = New_condition_tempo( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_ENTREE:
                          { $$ = New_condition_entree( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_BISTABLE:
                          { $$ = New_condition_bi( $1, alias, $4 );
                            break;
                          }
                         case MNEMO_MONOSTABLE:
                          { if ( (alias->barre && $1) || (!alias->barre && !$1))
                             { $$ = New_condition_mono( 0, alias, $4 ); }
                            else
                             { $$ = New_condition_mono( 1, alias, $4 ); }
                             break;
                          }
                         case MNEMO_HORLOGE:
                          { if ( (alias->barre && $1) || (!alias->barre && !$1))
                             { $$ = New_condition_horloge( 0, alias, $4 ); }
                            else
                             { $$ = New_condition_horloge( 1, alias, $4 ); }
                             break;
                          }
                         case MNEMO_ENTREE_ANA:
                          { char *chaine;
                            if ($5->type == T_EGAL)
                             { Emettre_erreur_new( "Ligne %d: '%s (EA%4d)' ne peut s'utiliser avec le comparateur '='",
                                                   DlsScanner_get_lineno(), $3, alias->num );
                               $$=New_chaine(2);
                               g_snprintf( $$, 2, "0" ); 
                             }
                            else
                             { taille = 50;
                               $$ = New_chaine( taille ); /* 10 caractères max */
                               switch($5->type)
                                { case INF        : g_snprintf( $$, taille, "EA_ech_inf(%f,%d)", $5->valf, alias->num ); break;
                                  case SUP        : g_snprintf( $$, taille, "EA_ech_sup(%f,%d)", $5->valf, alias->num ); break;
                                  case INF_OU_EGAL: g_snprintf( $$, taille, "EA_ech_inf_egal(%f,%d)", $5->valf, alias->num ); break;
                                  case SUP_OU_EGAL: g_snprintf( $$, taille, "EA_ech_sup_egal(%f,%d)", $5->valf, alias->num ); break;
                                }
                             }
                            break;
                          }
                         case MNEMO_REGISTRE:
                          { char *chaine;
                            taille = 40;
                            $$ = New_chaine( taille );
                            switch( $5->type )
                             { case INF        : g_snprintf( $$, taille, "R(%d)<%f", $3, $5 ); break;
                               case SUP        : g_snprintf( $$, taille, "R(%d)>%f", $3, $5 ); break;
                               case INF_OU_EGAL: g_snprintf( $$, taille, "R(%d)<=%f", $3, $5 ); break;
                               case SUP_OU_EGAL: g_snprintf( $$, taille, "R(%d)>=%f", $3, $5 ); break;
                               case T_EGAL     : g_snprintf( $$, taille, "R(%d)==%f", $3, $5 ); break;
                             }
                            break;
                           }
                         case MNEMO_CPT_IMP:
                          { taille = 30;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            switch($5->type)
                             { case INF        : g_snprintf( $$, taille, "CI(%d)<%f", alias->num, $5->valf );  break;
                               case SUP        : g_snprintf( $$, taille, "CI(%d)>%f", alias->num, $5->valf );  break;
                               case INF_OU_EGAL: g_snprintf( $$, taille, "CI(%d)<=%f", alias->num, $5->valf ); break;
                               case SUP_OU_EGAL: g_snprintf( $$, taille, "CI(%d)>=%f", alias->num, $5->valf ); break;
                               case T_EGAL     : g_snprintf( $$, taille, "CI(%d)==%f", alias->num, $5->valf ); break;
                             }
                            break;
                          }
                         default:
                          { Emettre_erreur_new( "Ligne %d: '%s' n'est pas une condition valide", DlsScanner_get_lineno(), $3 );
                            $$=New_chaine(2);
                            g_snprintf( $$, 2, "0" );
                          }
                       }
                    }
                   else { Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), acro );/* si l'alias n'existe pas */
                          $$=New_chaine(2);
                          g_snprintf( $$, 2, "0" );
                        }
                   if ($3) g_free($3);                                                   /* Libération du prefixe s'il existe */
                   g_free($2);                                                         /* On n'a plus besoin de l'identifiant */
                   Liberer_options($4);
                   if ($5) g_free($5);                                               /* Libération du comparateur s'il existe */
                }}
                ;

suffixe:          T_DPOINTS ID {{ $$=$2; }}
                | {{ $$=NULL; }}
                ;
/************************************************* Gestion des actions ********************************************************/
action:         action VIRGULE une_action
                {{ int taille;
                   $$=New_action();
                   taille = strlen($1->alors)+strlen($3->alors)+1;
                   $$->alors = New_chaine( taille );
                   g_snprintf( $$->alors, taille, "%s%s", $1->alors, $3->alors );
                   taille = 1;
                   if ($1->sinon) taille += strlen($1->sinon);
                   if ($3->sinon) taille += strlen($3->sinon);
                   if (taille>1)
                    { $$->sinon = New_chaine( taille );
                      if ($1->sinon && $3->sinon)
                       { g_snprintf( $$->sinon, taille, "%s%s", $1->sinon, $3->sinon ); }
                      else if ($1->sinon)
                       { g_snprintf( $$->sinon, taille, "%s", $1->sinon ); }
                      else
                       { g_snprintf( $$->sinon, taille, "%s", $3->sinon ); }
                    }
                   g_free($1->alors); if ($1->sinon) { g_free($1->sinon); }
                   g_free($3->alors); if ($3->sinon) { g_free($3->sinon); }
                   g_free($1); g_free($3);
                }}
                | une_action {{ $$=$1; }}
                ;

une_action:     ICONE ENTIER liste_options
                  {{ $$=New_action_icone($2, $3);
                     Liberer_options($3);
                  }}
                | CPT_H ENTIER liste_options
                  {{ $$=New_action_cpt_h($2, $3);
                     Liberer_options($3);
                  }}
                | T_CPT_IMP ENTIER liste_options
                  {{ $$=New_action_cpt_imp($2, $3);
                     Liberer_options($3);
                  }}
                | T_MSG ENTIER
                  {{ $$=New_action_msg($2); }}
                | T_ACT_COMOUT
                  {{ $$=New_action_vars_mono("vars->bit_comm_out"); }}
                | T_ACT_DEF
                  {{ $$=New_action_vars_mono("vars->bit_defaut"); }}
                | T_ACT_DEFF
                  {{ $$=New_action_vars_mono("vars->bit_defaut_fixe"); }}
                | T_ACT_ALA
                  {{ $$=New_action_vars_mono("vars->bit_alarme"); }}
                | T_ACT_ALAF
                  {{ $$=New_action_vars_mono("vars->bit_alarme_fixe"); }}
                | T_SBIEN_VEILLE
                  {{ $$=New_action_vars_mono("vars->bit_veille"); }}
                | T_SBIEN_ALE
                  {{ $$=New_action_vars_mono("vars->bit_alerte"); }}
                | T_SBIEN_ALEF
                  {{ $$=New_action_vars_mono("vars->bit_alerte_fixe"); }}
                | T_SPERS_DER
                  {{ $$=New_action_vars_mono("vars->bit_derangement"); }}
                | T_SPERS_DERF
                  {{ $$=New_action_vars_mono("vars->bit_derangement_fixe"); }}
                | T_SPERS_DAN
                  {{ $$=New_action_vars_mono("vars->bit_danger"); }}
                | T_SPERS_DANF
                  {{ $$=New_action_vars_mono("vars->bit_danger_fixe"); }}
                | barre ID suffixe liste_options
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   gchar *tech_id, *acro;
                   int taille;
                   if ($3) { tech_id = $2; acro = $3; }
                      else { tech_id = NULL; acro = $2; }
                   alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
                   if (!alias && $3) { alias = Set_new_external_alias(tech_id,acro); }/* Si dependance externe, on va chercher */
                   alias = Get_alias_par_acronyme(tech_id, acro);
                   if (!alias)
                    { char *chaine;
                      if ($3) Emettre_erreur_new( "Ligne %d: '%s:%s' is not defined", DlsScanner_get_lineno(), $2, $3 );
                         else Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), $2 );

                      $$=New_action();
                      taille = 2;
                      $$->alors = New_chaine( taille );
                      g_snprintf( $$->alors, taille, " " ); 
                      $$->sinon = NULL;
                    }
                   else                                                           /* L'alias existe, vérifions ses parametres */
                    { GList *options, *options_g, *options_d;
                      options_g = g_list_copy( $4 );
                      options_d = g_list_copy( alias->options );
                      options = g_list_concat( options_g, options_d );                  /* Concaténation des listes d'options */
                      if ($1 && (alias->bit==MNEMO_TEMPO ||
                                 alias->bit==MNEMO_MSG ||
                                 alias->bit==MNEMO_BUS ||
                                 alias->bit==MNEMO_MONOSTABLE)
                         )
                       { Emettre_erreur_new( "Ligne %d: '/%s' ne peut s'utiliser", DlsScanner_get_lineno(), alias->acronyme );
                         $$=New_action();
                         taille = 2;
                         $$->alors = New_chaine( taille );
                         g_snprintf( $$->alors, taille, " " ); 
                         $$->sinon = NULL;
                       }
                      else switch(alias->bit)
                       { case MNEMO_TEMPO : $$=New_action_tempo( alias, options ); break;
                         case MNEMO_MSG   : $$=New_action_msg_by_alias( alias );   break;
                         case MNEMO_BUS   : $$=New_action_bus( alias, options );   break;
                         case MNEMO_SORTIE: $$=New_action_sortie( alias, $1, options );  break;
                         case MNEMO_BISTABLE:
                                    if (alias->num >= NBR_BIT_BISTABLE_RESERVED || alias->type==ALIAS_TYPE_DYNAMIC)
                                     { $$=New_action_bi_by_alias( alias, $1 ); }
                                    else
                                     { Emettre_erreur_new( "Ligne %d: 'B%04d' could not be set (system bit)", DlsScanner_get_lineno(), alias->num );
                                       $$=New_action();
                                       taille = 2;
                                       $$->alors = New_chaine( taille );
                                       g_snprintf( $$->alors, taille, " " ); 
                                       $$->sinon = NULL;
                                     }
                                    break;
                         case MNEMO_MONOSTABLE: $$=New_action_mono_by_alias( alias );         break;
                         case MNEMO_CPTH      : $$=New_action_cpt_h( alias->num, options );   break;
                         case MNEMO_CPT_IMP   : $$=New_action_cpt_imp( alias->num, options ); break;
                         case MNEMO_MOTIF     : $$=New_action_icone( alias->num, options );   break;
                         default: { Emettre_erreur_new( "Ligne %d: '%s:%s' syntax error", DlsScanner_get_lineno(),
                                                        alias->tech_id, alias->acronyme );
                                    $$=New_action();
                                    taille = 2;
                                    $$->alors = New_chaine( taille );
                                    g_snprintf( $$->alors, taille, " " ); 
                                    $$->sinon = NULL;
                                  }
                       }
                      g_list_free(options);
                    }
                   Liberer_options($4);                                                    /* On libére les options "locales" */
                   if ($3) g_free($3);
                   g_free($2);
                }}
                ;

comparateur:    ordre VALF
                {{ $$ = New_comparateur();
                   $$->type = $1;
                   $$->valf = $2;
                }}
                |     {{ $$=NULL; }}
                ;

barre:          BARRE {{ $$=1; }}
                |     {{ $$=0; }}
                ;
modulateur:     APRES        {{ $$=APRES;  }}
                | AVANT      {{ $$=AVANT;  }}
                |            {{ $$=0;      }}
                ;
jour_semaine:   LUNDI        {{ $$=1; }}
                | MARDI      {{ $$=2; }}
                | MERCREDI   {{ $$=3; }}
                | JEUDI      {{ $$=4; }}
                | VENDREDI   {{ $$=5; }}
                | SAMEDI     {{ $$=6; }}
                | DIMANCHE   {{ $$=0; }}
                ;
ordre:          INF | SUP | INF_OU_EGAL | SUP_OU_EGAL | T_EGAL
                ;
/**************************************************** Gestion des options *****************************************************/
liste_options:  T_POUV options T_PFERM   {{ $$ = $2;   }}
                |                        {{ $$ = NULL; }}
                ;

options:        options VIRGULE une_option
                {{ $$ = g_list_append( $1, $3 );
                }}
                | une_option {{ $$ = g_list_append( NULL, $1 ); }}
                ;

une_option:     MODE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = MODE;
                   $$->entier = $3;
                }}
                | COLOR T_EGAL couleur
                {{ $$=New_option();
                   $$->type = COLOR;
                   $$->entier = $3;
                }}
                | T_LIBELLE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_LIBELLE;
                   $$->chaine = $3;
                }}
                | T_ETIQUETTE T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_ETIQUETTE;
                   $$->chaine = $3;
                }}
                | CLIGNO
                {{ $$=New_option();
                   $$->type = CLIGNO;
                   $$->entier = 1;
                }}
                | CLIGNO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = CLIGNO;
                   $$->entier = $3;
                }}
                | CONSIGNE T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = CONSIGNE;
                   $$->entier = $3;
                }}
                | RESET
                {{ $$=New_option();
                   $$->type = RESET;
                   $$->entier = 1;
                }}
                | RESET T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RESET;
                   $$->entier = $3;
                }}
                | RATIO T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RATIO;
                   $$->entier = $3;
                }}
                | T_EDGE_UP
                {{ $$=New_option();
                   $$->type = T_EDGE_UP;
                   $$->entier = 1;
                }}
                | T_DAA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DAA;
                   $$->entier = $3;
                }}
                | T_DMINA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DMINA;
                   $$->entier = $3;
                }}
                | T_DMAXA T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DMAXA;
                   $$->entier = $3;
                }}
                | T_DAD T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_DAD;
                   $$->entier = $3;
                }}
                | T_RANDOM T_EGAL ENTIER
                {{ $$=New_option();
                   $$->type = T_RANDOM;
                   $$->entier = $3;
                }}
                | T_TYPE T_EGAL type_msg
                {{ $$=New_option();
                   $$->type = T_TYPE;
                   $$->entier = $3;
                }}
                | T_HOST T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_HOST;
                   $$->chaine = $3;
                }}
                | T_THREAD T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_THREAD;
                   $$->chaine = $3;
                }}
                | T_TAG T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_TAG;
                   $$->chaine = $3;
                }}
                | T_PARAM1 T_EGAL T_CHAINE
                {{ $$=New_option();
                   $$->type = T_PARAM1;
                   $$->chaine = $3;
                }}
                ;


couleur:        ROUGE | VERT | BLEU | JAUNE | NOIR | BLANC | GRIS | ORANGE | KAKI
                ;
type_msg:         T_ETAT    {{ $$=MSG_ETAT; }}
                | T_ATTENTE {{ $$=MSG_ATTENTE; }}
                | T_DEFAUT  {{ $$=MSG_DEFAUT; }}
                | T_ALARME  {{ $$=MSG_ALARME; }}
                | T_VEILLE  {{ $$=MSG_VEILLE; }}
                | T_ALERTE  {{ $$=MSG_ALERTE; }}
                | T_DANGER  {{ $$=MSG_DANGER; }}
                | T_DERANGEMENT {{ $$=MSG_DERANGEMENT; }}
                ;

%%

/*----------------------------------------------------------------------------------------------------------------------------*/
