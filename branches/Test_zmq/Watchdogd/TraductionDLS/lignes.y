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

%token <val>    PVIRGULE VIRGULE DONNE EQUIV DPOINT MOINS T_POUV T_PFERM T_EGAL OU ET BARRE T_FOIS
%token <val>    MODE CONSIGNE COLOR CLIGNO RESET RATIO

%token <val>    INF SUP INF_OU_EGAL SUP_OU_EGAL T_TRUE T_FALSE
%type  <val>    ordre

%token <val>    HEURE APRES AVANT LUNDI MARDI MERCREDI JEUDI VENDREDI SAMEDI DIMANCHE
%type  <val>    modulateur jour_semaine

%token <val>    BI MONO ENTREE SORTIE T_TEMPO T_TYPE T_RETARD
%token <val>    T_MSG ICONE CPT_H T_CPT_IMP EANA T_START T_REGISTRE
%type  <val>    alias_bit

%token <val>    ROUGE VERT BLEU JAUNE NOIR BLANC ORANGE GRIS KAKI T_EDGE_UP
%type  <val>    couleur

%token <chaine> ID
%token <val>    ENTIER
%token <valf>   VALF

%type  <val>         barre calcul_ea_result
%type  <gliste>      liste_options options
%type  <option>      une_option
%type  <chaine>      unite facteur expr
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

/************************************************* Gestion des alias **************************************/
listeAlias:     un_alias listeAlias
                | un_alias
                ;
                
un_alias:       ID EQUIV barre alias_bit ENTIER liste_options PVIRGULE
                {{ char *chaine;
                   int taille;
                   switch($4)
                    { case ENTREE:
                      case SORTIE:
                      case BI    : if ( New_alias($1, $4, $5, $3, $6) == FALSE )                             /* Deja defini ? */
                                    { Emettre_erreur_new( "Ligne %d: '%s' is already defined", DlsScanner_get_lineno(), $1 ); }
                                   break;
                      case T_TEMPO :
                      case EANA  :
                      case MONO  :
                      case CPT_H :
                      case T_CPT_IMP:
                      case T_MSG :
                      case T_REGISTRE :
                      case ICONE : if ($3==1)                                             /* Barre = 1 ?? */
                                    { Emettre_erreur_new( "Ligne %d:Use of '/%s' is forbidden", DlsScanner_get_lineno(), $1 ); }
                                   else
                                    { if (New_alias($1, $4, $5, 0, $6) == FALSE)
                                       { Emettre_erreur_new( "Ligne %d: '%s' is already defined", DlsScanner_get_lineno(), $1 ); }
                                    }
                                   break;
                      default: Emettre_erreur_new( "Ligne %d: Syntaxe Error near '%s'", DlsScanner_get_lineno(), $1 );
                               break;
                    }
                }}
                ;
alias_bit:      BI | MONO | ENTREE | SORTIE | T_MSG | T_TEMPO | ICONE | CPT_H | T_CPT_IMP | EANA | T_REGISTRE
                ;
/**************************************************** Gestion des instructions ************************************************/
listeInstr:     une_instr listeInstr
                | une_instr
                ;

une_instr:      MOINS expr DONNE action PVIRGULE
                {{ int taille;
                   char *instr;
                   taille = strlen($2)+strlen($4->alors)+11;
                   if ($4->sinon)
                    { taille += (strlen($4->sinon) + 10);
                      instr = New_chaine( taille );
                      g_snprintf( instr, taille, "if(%s) { %s }\nelse { %s }\n", $2, $4->alors, $4->sinon );
                    }
                   else
                    { instr = New_chaine( taille );
                      g_snprintf( instr, taille, "if(%s) { %s }\n", $2, $4->alors );
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
                   alias = Get_alias_par_nom($1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case EANA  :
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "EA_ech(%d)", alias->num );
                            break;
                          }
                         case T_REGISTRE:
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "R(%d)", alias->num );
                            break;
                          }
                         case T_CPT_IMP:
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
                   Check_ownership ( MNEMO_REGISTRE, $2 );
                }}
                | ID
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_nom($1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case T_REGISTRE:
                          { $$ = alias->num;
                            Check_ownership ( MNEMO_REGISTRE, alias->num );
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
                   taille = 10;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(start)" );
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
                | barre BI ENTIER liste_options
                {{ $$ = New_condition_bi ( $1, $3, $4 );
                   Liberer_options($4);
                }}
                | barre MONO ENTIER
                {{ int taille;
                   taille = 10;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   if ($1) g_snprintf( $$, taille, "!M(%d)", $3 );
                   else g_snprintf( $$, taille, "M(%d)", $3 );
                }}
                | barre ENTREE ENTIER liste_options
                {{ $$ = New_condition_entree ( $1, $3, $4 );
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
                | barre T_TEMPO ENTIER liste_options 
                {{ int taille;
                   taille = 40;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   g_snprintf( $$, taille, "%sT(%d)",
                               ($1==1 ? "!" : ""), $3 );
                   Liberer_options($4);
                }}
                | barre T_POUV expr T_PFERM
                {{ int taille;
                   taille = strlen($3)+5;
                   $$ = New_chaine( taille );
                   if ($1) { g_snprintf( $$, taille, "!(%s)", $3 ); }
                   else    { g_snprintf( $$, taille, "(%s)", $3 ); }
                   g_free($3);
                }}
                | barre ID liste_options comparateur
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_nom($2);                                                      /* On recupere l'alias */
                   if (alias)
                    { if ($4 && (alias->bit==T_TEMPO ||                              /* Vérification des bits non comparables */
                                 alias->bit==ENTREE ||
                                 alias->bit==BI ||
                                 alias->bit==MONO)
                         )
                       { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser dans une comparaison", DlsScanner_get_lineno(), $2 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );                                      
                       } else
                      if (!$4 && (alias->bit==EANA ||                    /* Vérification des bits obligatoirement comparables */
                                  alias->bit==T_REGISTRE ||
                                  alias->bit==T_CPT_IMP ||
                                  alias->bit==CPT_H)
                         )
                       { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser qu'avec une comparaison", DlsScanner_get_lineno(), $2 );
                         $$=New_chaine(2);
                         g_snprintf( $$, 2, "0" );                                      
                       } 
                      else switch(alias->bit)                              /* On traite que ce qui peut passer en "condition" */
                       { case T_TEMPO :
                          { taille = 40;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            g_snprintf( $$, taille, "%sT(%d)", ($1==1 ? "!" : ""), alias->num );
                            break;
                          }
                         case ENTREE:
                          { if ( (alias->barre && $1) || (!alias->barre && !$1))
                             { $$ = New_condition_entree( 0, alias->num, $3 ); }
                            else
                             { $$ = New_condition_entree( 1, alias->num, $3 ); }
                            break;
                          }
                         case BI:
                          { if ( (alias->barre && $1) || (!alias->barre && !$1))
                             { $$ = New_condition_bi( 0, alias->num, $3 ); }
                            else
                             { $$ = New_condition_bi( 1, alias->num, $3 ); }
                            break;
                          }
                         case MONO:
                          { taille = 15;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            if ( (!$1 && !alias->barre) || ($1 && alias->barre) )
                                 { g_snprintf( $$, taille, "M(%d)", alias->num ); }
                            else { g_snprintf( $$, taille, "!M(%d)", alias->num ); }
                            break;
                          }
                         case EANA:
                          { char *chaine;
                            if ($4->type == T_EGAL)
                             { Emettre_erreur_new( "Ligne %d: '%s (EA%4d)' ne peut s'utiliser avec le comparateur '='",
                                                   DlsScanner_get_lineno(), $2, alias->num );
                               $$=New_chaine(2);
                               g_snprintf( $$, 2, "0" ); 
                             }
                            else
                             { taille = 50;
                               $$ = New_chaine( taille ); /* 10 caractères max */
                               switch($4->type)
                                { case INF        : g_snprintf( $$, taille, "EA_ech_inf(%f,%d)", $4->valf, alias->num ); break;
                                  case SUP        : g_snprintf( $$, taille, "EA_ech_sup(%f,%d)", $4->valf, alias->num ); break;
                                  case INF_OU_EGAL: g_snprintf( $$, taille, "EA_ech_inf_egal(%f,%d)", $4->valf, alias->num ); break;
                                  case SUP_OU_EGAL: g_snprintf( $$, taille, "EA_ech_sup_egal(%f,%d)", $4->valf, alias->num ); break;
                                }
                             }
                            break;
                          }
                         case T_REGISTRE:
                          { char *chaine;
                            taille = 40;
                            $$ = New_chaine( taille );
                            switch( $4->type )
                             { case INF        : g_snprintf( $$, taille, "R(%d)<%f", $2, $4 ); break;
                               case SUP        : g_snprintf( $$, taille, "R(%d)>%f", $2, $4 ); break;
                               case INF_OU_EGAL: g_snprintf( $$, taille, "R(%d)<=%f", $2, $4 ); break;
                               case SUP_OU_EGAL: g_snprintf( $$, taille, "R(%d)>=%f", $2, $4 ); break;
                               case T_EGAL     : g_snprintf( $$, taille, "R(%d)==%f", $2, $4 ); break;
                             }
                            break;
                           }
                         case T_CPT_IMP:
                          { taille = 30;
                            $$ = New_chaine( taille ); /* 10 caractères max */
                            switch($4->type)
                             { case INF        : g_snprintf( $$, taille, "CI(%d)<%f", alias->num, $4->valf );  break;
                               case SUP        : g_snprintf( $$, taille, "CI(%d)>%f", alias->num, $4->valf );  break;
                               case INF_OU_EGAL: g_snprintf( $$, taille, "CI(%d)<=%f", alias->num, $4->valf ); break;
                               case SUP_OU_EGAL: g_snprintf( $$, taille, "CI(%d)>=%f", alias->num, $4->valf ); break;
                               case T_EGAL     : g_snprintf( $$, taille, "CI(%d)==%f", alias->num, $4->valf ); break;
                             }
                            break;
                          }
                         default:
                          { Emettre_erreur_new( "Ligne %d: '%s' n'est pas une condition valide", DlsScanner_get_lineno(), $2 );
                            $$=New_chaine(2);
                            g_snprintf( $$, 2, "0" );
                          }
                       }
                    }
                   else { Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), $2 );/* si l'alias n'existe pas */
                          $$=New_chaine(2);
                          g_snprintf( $$, 2, "0" );
                        }
                   g_free($2);                                                         /* On n'a plus besoin de l'identifiant */
                   Liberer_options($3);
                   if ($4) g_free($4);                                               /* Libération du comparateur s'il existe */
                }}
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

une_action:     barre SORTIE ENTIER
                  {{ $$=New_action_sortie($3, $1);     }}
                | barre BI ENTIER
                  {{ if ($3 >= NBR_BIT_BISTABLE_RESERVED)
                       { $$=New_action_bi($3, $1); }
                     else
                       { guint taille;
                         Emettre_erreur_new( "Ligne %d: 'B%04d' could not be set (system bit)", DlsScanner_get_lineno(), $3 );
                         $$=New_action();
                         taille = 2;
                         $$->alors = New_chaine( taille );
                         g_snprintf( $$->alors, taille, " " ); 
                         $$->sinon = NULL;
                       }
                  }}
                | MONO ENTIER
                  {{ $$=New_action_mono($2);           }}
                | ICONE ENTIER liste_options
                  {{ $$=New_action_icone($2, $3);
                     Liberer_options($3);
                  }}
                | T_TEMPO ENTIER liste_options
                  {{ $$=New_action_tempo($2, $3);
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
                | barre ID liste_options
                {{ struct ALIAS *alias;                                                   /* Definition des actions via alias */
                   int taille;
                   alias = Get_alias_par_nom( $2 );
                   if (!alias)
                    { char *chaine;
                      Emettre_erreur_new( "Ligne %d: '%s' is not defined", DlsScanner_get_lineno(), $2 );

                      $$=New_action();
                      taille = 2;
                      $$->alors = New_chaine( taille );
                      g_snprintf( $$->alors, taille, " " ); 
                      $$->sinon = NULL;
                    }
                   else                                                           /* L'alias existe, vérifions ses parametres */
                    { GList *options, *options_g, *options_d;
                      options_g = g_list_copy( $3 );
                      options_d = g_list_copy( alias->options );
                      options = g_list_concat( options_g, options_d );                  /* Concaténation des listes d'options */
                      if ($1 && (alias->bit==T_TEMPO ||
                                 alias->bit==T_MSG ||
                                 alias->bit==MONO)
                         )
                       { Emettre_erreur_new( "Ligne %d: '/%s' ne peut s'utiliser", DlsScanner_get_lineno(), alias->nom );
                         $$=New_action();
                         taille = 2;
                         $$->alors = New_chaine( taille );
                         g_snprintf( $$->alors, taille, " " ); 
                         $$->sinon = NULL;
                       }
                      else switch(alias->bit)
                       { case T_TEMPO: $$=New_action_tempo( alias->num, options );   break;
                         case T_MSG  : $$=New_action_msg( alias->num );              break;
                         case SORTIE : $$=New_action_sortie( alias->num, $1 );       break;
                         case BI     : if (alias->num >= NBR_BIT_BISTABLE_RESERVED)
                                        { $$=New_action_bi( alias->num, $1 ); }
                                       else
                                        { Emettre_erreur_new( "Ligne %d: 'B%04d' could not be set (system bit)", DlsScanner_get_lineno(), alias->bit );
                                          $$=New_action();
                                          taille = 2;
                                          $$->alors = New_chaine( taille );
                                          g_snprintf( $$->alors, taille, " " ); 
                                          $$->sinon = NULL;
                                        }
                                       break;
                         case MONO   : $$=New_action_mono( alias->num );        break;
                         case CPT_H  : $$=New_action_cpt_h( alias->num, options );   break;
                         case T_CPT_IMP: $$=New_action_cpt_imp( alias->num, options ); break;
                         case ICONE  : $$=New_action_icone( alias->num, options );   break;
                         default: { Emettre_erreur_new( "Ligne %d: '%s' syntax error", DlsScanner_get_lineno(), alias->nom );
                                    $$=New_action();
                                    taille = 2;
                                    $$->alors = New_chaine( taille );
                                    g_snprintf( $$->alors, taille, " " ); 
                                    $$->sinon = NULL;
                                  }
                       }
                      g_list_free(options);
                    }
                   Liberer_options($3);                                                    /* On libére les options "locales" */
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
                ;


couleur:        ROUGE | VERT | BLEU | JAUNE | NOIR | BLANC | GRIS | ORANGE | KAKI
                ;
%%

/*----------------------------------------------------------------------------------------------------------------------------*/
