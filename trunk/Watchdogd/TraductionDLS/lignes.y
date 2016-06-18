/**********************************************************************************************************/
/* Watchdogd/TraductionDLS/ligne.y        Définitions des ligne dls DLS                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                    jeu. 24 juin 2010 19:37:44 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

extern int ligne_source_dls;                           /* Compteur du numero de ligne_source_dls en cours */
int erreur;                                                             /* Compteur d'erreur du programme */
#define  NON_DEFINI                  "Ligne %d: %s is not defined\n"
#define  DEJA_DEFINI                 "Ligne %d: %s is already defined\n"
#define  INTERDIT_GAUCHE             "Ligne %d: %s interdit en position gauche\n"
#define  INTERDIT_DROITE             "Ligne %d: %s interdit en position droite\n"
#define  INTERDIT_BARRE              "Ligne %d: Use of /%s forbidden\n"
#define  INTERDIT_BARRE_DROITE       "Ligne %d: /%s interdit en position droite\n"
#define  INTERDIT_REL_ORDRE          "Ligne %d: %s interdit dans la relation d'ordre\n"
#define  INTERDIT_COMPARAISON        "Ligne %d: %s ne peut s'utiliser dans une comparaison\n"
#define  INTERDIT_CALCUL             "Ligne %d: %s ne peut s'utiliser dans un calcul\n"
#define  INTERDIT_CALCUL_RESULT      "Ligne %d: %s ne peut s'utiliser dans un résultat de calcul\n"
#define  INTERDIT_BIT_B_RESERVED     "Ligne %d: Bistable système B%04d interdit en position droite\n"
#define  MANQUE_COMPARAISON          "Ligne %d: %s doit s'utiliser dans une comparaison\n"
#define  ERR_SYNTAXE                 "Ligne %d: Erreur de syntaxe -> %s\n"


%}

%union { int val;
         float valf;
         char *chaine;
         GList *gliste;
         struct OPTION *option;
         struct ACTION *action;
         struct COMPARATEUR *comparateur;
       };

%token <val>    PVIRGULE VIRGULE DONNE EQUIV DPOINT MOINS POUV PFERM EGAL OU ET BARRE T_FOIS
%token <val>    MODE CONSIGNE COLOR CLIGNO RESET RATIO

%token <val>    INF SUP INF_OU_EGAL SUP_OU_EGAL T_TRUE T_FALSE
%type  <val>    ordre

%token <val>    HEURE APRES AVANT LUNDI MARDI MERCREDI JEUDI VENDREDI SAMEDI DIMANCHE
%type  <val>    modulateur jour_semaine

%token <val>    BI MONO ENTREE SORTIE T_TEMPO T_TYPE T_RETARD
%token <val>    T_MSG ICONE CPT_H CPT_IMP EANA T_START
%type  <val>    alias_bit

%token <val>    ROUGE VERT BLEU JAUNE NOIR BLANC ORANGE GRIS KAKI
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
                      case BI    : if ( New_alias($1, $4, $5, $3, $6) == FALSE )         /* Deja defini ? */
                                    { taille = strlen($1) + strlen(DEJA_DEFINI) + 1;
                                      chaine = New_chaine(taille);
                                      g_snprintf( chaine, taille, DEJA_DEFINI, ligne_source_dls, $1 );
                                      Emettre_erreur(chaine); g_free(chaine);
                                      erreur++;
                                    }
                                   break;
                      case T_TEMPO :
                      case EANA  :
                      case MONO  :
                      case CPT_H :
                      case CPT_IMP:
                      case T_MSG :
                      case ICONE : if ($3==1)                                             /* Barre = 1 ?? */
                                    { taille = strlen($1) + strlen(INTERDIT_BARRE) + 1;
                                      chaine = New_chaine(taille);
                                      g_snprintf( chaine, taille, INTERDIT_BARRE, ligne_source_dls, $1 );
                                      Emettre_erreur(chaine); g_free(chaine);
                                      erreur++;
                                    }
                                   else
                                    { if (New_alias($1, $4, $5, 0, $6) == FALSE)
                                       { taille = strlen($1) + strlen(DEJA_DEFINI) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf( chaine, taille, DEJA_DEFINI, ligne_source_dls, $1 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                       }
                                    }
                                   break;
                      default: taille = strlen($1) + strlen(ERR_SYNTAXE) + 1;
                               chaine = New_chaine(taille);
                               g_snprintf( chaine, taille, ERR_SYNTAXE, ligne_source_dls, $1 );
                               Emettre_erreur(chaine); g_free(chaine);
                               erreur++;
                               break;
                    }
                }}
                ;
alias_bit:      BI | MONO | ENTREE | SORTIE | T_MSG | T_TEMPO | ICONE | CPT_H | CPT_IMP | EANA
                ;
/******************************************* Gestion des instructions *************************************/
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
                | MOINS expr MOINS calcul_expr DONNE calcul_ea_result PVIRGULE
                {{ int taille;
                   char *instr;
                   taille = strlen($4)+strlen($2)+35;
                   instr = New_chaine( taille );
                   g_snprintf( instr, taille, "if(%s) { SEA(%d,%s); }\n", $2, $6, $4 );
                   Emettre( instr ); g_free(instr);
                   g_free($2);
                   g_free($4);
                }}
                ;
/******************************************* Partie CALCUL ************************************************/
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
                | POUV calcul_expr PFERM
                {{ $$=$2; }}
                | ID
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_nom($1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case EANA  : { taille = 50;
                                        $$ = New_chaine( taille ); /* 10 caractères max */
                                        g_snprintf( $$, taille, "EA_ech(%d)", alias->num ); break;
                                      }
                                      break;
                         default:     taille = strlen($1) + strlen(INTERDIT_CALCUL) + 1;
                                      chaine = New_chaine(taille);
                                      g_snprintf(chaine, taille, INTERDIT_CALCUL, ligne_source_dls, $1 );
                                      Emettre_erreur(chaine); g_free(chaine);
                                      erreur++;
                                      $$=New_chaine(2);
                                      g_snprintf( $$, 2, "0" );
                       }
                    }
                   else { taille = strlen($1) + strlen(NON_DEFINI) + 1;
                          chaine = New_chaine(taille);
                          g_snprintf(chaine, taille, NON_DEFINI, ligne_source_dls, $1 );
                          Emettre_erreur(chaine); g_free(chaine);
                          erreur++;
                          
                          $$=New_chaine(2);
                          g_snprintf( $$, 2, "0" );
                        }
                   g_free($1);                                     /* On n'a plus besoin de l'identifiant */
                }}
                ;

calcul_ea_result: EANA ENTIER
                {{ $$ = $2;
                }}
                | ID
                {{ struct ALIAS *alias;
                   char *chaine;
                   int taille;
                   alias = Get_alias_par_nom($1);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case EANA  : $$ = alias->num;
                                      break;
                         default:     taille = strlen($1) + strlen(INTERDIT_CALCUL_RESULT) + 1;
                                      chaine = New_chaine(taille);
                                      g_snprintf(chaine, taille, INTERDIT_CALCUL_RESULT, ligne_source_dls, $1 );
                                      Emettre_erreur(chaine); g_free(chaine);
                                      erreur++;
                                      $$=0;
                       }
                    }
                   else { taille = strlen($1) + strlen(NON_DEFINI) + 1;
                          chaine = New_chaine(taille);
                          g_snprintf(chaine, taille, NON_DEFINI, ligne_source_dls, $1 );
                          Emettre_erreur(chaine); g_free(chaine);
                          erreur++;
                          $$=0;
                        }
                   g_free($1);                                     /* On n'a plus besoin de l'identifiant */
                }}
                ;
/******************************************* Partie LOGIQUE ***********************************************/
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
                   taille = 18;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(start)" );
                }}
                | T_TRUE
                {{ int taille;
                   taille = 18;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(1)" );
                }}
                | T_FALSE
                {{ int taille;
                   taille = 18;
                   $$ = New_chaine(taille);
                   g_snprintf( $$, taille, "(0)" );
                }}
                | barre BI ENTIER
                {{ int taille;
                   taille = 10;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   if ($1) g_snprintf( $$, taille, "!B(%d)", $3 );
                   else g_snprintf( $$, taille, "B(%d)", $3 );
                }}
                | barre MONO ENTIER
                {{ int taille;
                   taille = 10;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   if ($1) g_snprintf( $$, taille, "!M(%d)", $3 );
                   else g_snprintf( $$, taille, "M(%d)", $3 );
                }}
                | barre ENTREE ENTIER
                {{ int taille;
                   taille = 10;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   if ($1) g_snprintf( $$, taille, "!E(%d)", $3 );
                   else g_snprintf( $$, taille, "E(%d)", $3 );
                }}
               | EANA ENTIER ordre VALF
                {{ int taille;
                   taille = 40;
                   $$ = New_chaine( taille );
                   switch( $3 )
                    { case INF        : g_snprintf( $$, taille, "EA_ech_inf(%f,%d)", $4, $2 ); break;
                      case SUP        : g_snprintf( $$, taille, "EA_ech_sup(%f,%d)", $4, $2 ); break;
                      case INF_OU_EGAL: g_snprintf( $$, taille, "EA_ech_inf_egal(%f,%d)", $4, $2 ); break;
                      case SUP_OU_EGAL: g_snprintf( $$, taille, "EA_ech_sup_egal(%f,%d)", $4, $2 ); break;
                    }
                }}
                | CPT_IMP ordre VALF
                {{ int taille;
                   taille = 30;
                   $$ = New_chaine( taille ); /* 10 caractères max */
                   switch( $1 )
                    { case INF        : g_snprintf( $$, taille, "CI(%d)<%f", $1, $3 );  break;
                      case SUP        : g_snprintf( $$, taille, "CI(%d)>%f", $1, $3 );  break;
                      case INF_OU_EGAL: g_snprintf( $$, taille, "CI(%d)<=%f", $1, $3 ); break;
                      case SUP_OU_EGAL: g_snprintf( $$, taille, "CI(%d)>=%f", $1, $3 ); break;
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
                | barre POUV expr PFERM
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
                   alias = Get_alias_par_nom($2);                                  /* On recupere l'alias */
                   if (alias)
                    { switch(alias->bit)               /* On traite que ce qui peut passer en "condition" */
                       { case T_TEMPO : if ($4)
                                       { taille = strlen($2) + strlen(INTERDIT_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, INTERDIT_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                         $$=New_chaine(2);
                                         g_snprintf( $$, 2, "0" );                                      
                                       } 
                                      else
                                       { int taille;
                                         taille = 40;
                                         $$ = New_chaine( taille ); /* 10 caractères max */
                                         g_snprintf( $$, taille, "%sT(%d)",
                                                     ($1==1 ? "!" : ""), alias->num );
                                       }
                                      break;
                         case ENTREE: if ($4)
                                       { taille = strlen($2) + strlen(INTERDIT_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, INTERDIT_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                         $$=New_chaine(2);
                                         g_snprintf( $$, 2, "0" );                                      
                                       }
                                      else
                                       { taille = 15;
                                         $$ = New_chaine( taille ); /* 10 caractères max */
                                         if ( (!$1 && !alias->barre) || ($1 && alias->barre) )
                                              g_snprintf( $$, taille, "E(%d)", alias->num );
                                         else g_snprintf( $$, taille, "!E(%d)", alias->num );
                                       }
                                      break;
                         case BI    : if ($4)
                                       { taille = strlen($2) + strlen(INTERDIT_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, INTERDIT_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                         $$=New_chaine(2);
                                         g_snprintf( $$, 2, "0" );                                      
                                       }
                                      else
                                       { taille = 15;
                                         $$ = New_chaine( taille ); /* 10 caractères max */
                                         if ( (!$1 && !alias->barre) || ($1 && alias->barre) )
                                              g_snprintf( $$, taille, "B(%d)", alias->num );
                                         else g_snprintf( $$, taille, "!B(%d)", alias->num );
                                       }
                                      break;
                         case MONO  : if ($4)
                                       { taille = strlen($2) + strlen(INTERDIT_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, INTERDIT_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                         $$=New_chaine(2);
                                         g_snprintf( $$, 2, "0" );                                      
                                       }
                                      else
                                       { taille = strlen(alias->nom)+2;
                                         $$ = New_chaine( taille ); /* 10 caractères max */
                                         if ( (!$1 && !alias->barre) || ($1 && alias->barre) )
                                              g_snprintf( $$, taille, "M(%d)", alias->num );
                                         else g_snprintf( $$, taille, "!M(%d)", alias->num );
                                       }
                                      break;
                         case EANA  : if (!$4)
                                       { taille = strlen($2) + strlen(MANQUE_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, MANQUE_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
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
                         case CPT_IMP:if (!$4)
                                       { taille = strlen($2) + strlen(MANQUE_COMPARAISON) + 1;
                                         chaine = New_chaine(taille);
                                         g_snprintf(chaine, taille, MANQUE_COMPARAISON, ligne_source_dls, $2 );
                                         Emettre_erreur(chaine); g_free(chaine);
                                         erreur++;
                                         $$=New_chaine(2);
                                         g_snprintf( $$, 2, "0" );                                      
                                       }
                                      else
                                       { taille = 30;
                                         $$ = New_chaine( taille ); /* 10 caractères max */
                                         switch($4->type)
                                          { case INF        : g_snprintf( $$, taille, "CI(%d)<%f", alias->num, $4->valf );  break;
                                            case SUP        : g_snprintf( $$, taille, "CI(%d)>%f", alias->num, $4->valf );  break;
                                            case INF_OU_EGAL: g_snprintf( $$, taille, "CI(%d)<=%f", alias->num, $4->valf ); break;
                                            case SUP_OU_EGAL: g_snprintf( $$, taille, "CI(%d)>=%f", alias->num, $4->valf ); break;
                                          }
                                       }
                                      break;
                         default:     taille = strlen($2) + strlen(INTERDIT_GAUCHE) + 1;
                                      chaine = New_chaine(taille);
                                      g_snprintf(chaine, taille, INTERDIT_GAUCHE, ligne_source_dls, $2 );
                                      Emettre_erreur(chaine); g_free(chaine);
                                      erreur++;
                                      $$=New_chaine(2);
                                      g_snprintf( $$, 2, "0" );
                       }
                    }
                   else { taille = strlen($2) + strlen(NON_DEFINI) + 1;
                          chaine = New_chaine(taille);
                          g_snprintf(chaine, taille, NON_DEFINI, ligne_source_dls, $2 );
                          Emettre_erreur(chaine); g_free(chaine);
                          erreur++;
                          
                          $$=New_chaine(2);
                          g_snprintf( $$, 2, "0" );
                        }
                   g_free($2);                                     /* On n'a plus besoin de l'identifiant */
                   Liberer_options($3);
                   if ($4) g_free($4);
                }}
                ;
/********************************************* Gestion des actions ****************************************/
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

une_action:     barre SORTIE ENTIER           {{ $$=New_action_sortie($3, $1);     }}
                | barre BI ENTIER
                  {{ if ($3 >= NBR_BIT_BISTABLE_RESERVED)
                       { $$=New_action_bi($3, $1); }
                     else
                       { char *chaine;
                          guint taille;
                         taille = strlen(INTERDIT_BIT_B_RESERVED) + 1;
                         chaine = New_chaine(taille);
                         g_snprintf( chaine, taille, INTERDIT_BIT_B_RESERVED, ligne_source_dls, $3 );
                         Emettre_erreur(chaine); g_free(chaine);
                         erreur++;

                         $$=New_action();
                         taille = 2;
                         $$->alors = New_chaine( taille );
                         g_snprintf( $$->alors, taille, " " ); 
                         $$->sinon = NULL;
                       }
                  }}
                | MONO ENTIER                 {{ $$=New_action_mono($2);           }}
                | ICONE ENTIER liste_options
                  {{ $$=New_action_icone($2, $3);
                     Liberer_options($3);
                  }}
                | T_TEMPO ENTIER liste_options
                  {{ $$=New_action_tempo($2, $3);
                     Liberer_options($3);
                  }}
                | CPT_H ENTIER                {{ $$=New_action_cpt_h($2);          }}
                | CPT_IMP ENTIER liste_options
                  {{ $$=New_action_cpt_imp($2, $3);
                     Liberer_options($3);
                  }}
                | T_MSG ENTIER                {{ $$=New_action_msg($2);            }}
                | barre ID liste_options
                {{ struct ALIAS *alias;                               /* Definition des actions via alias */
                   int taille;
                   alias = Get_alias_par_nom( $2 );
                   if (!alias)
                    { char *chaine;
                      taille = strlen($2) + strlen(NON_DEFINI) + 1;
                      chaine = New_chaine(taille);
                      g_snprintf( chaine, taille, NON_DEFINI, ligne_source_dls, $2 );
                      Emettre_erreur(chaine); g_free(chaine);
                      erreur++;

                      $$=New_action();
                      taille = 2;
                      $$->alors = New_chaine( taille );
                      g_snprintf( $$->alors, taille, " " ); 
                      $$->sinon = NULL;
                    }
                   else
                    { GList *options, *options_g, *options_d;
                      options_g = g_list_copy( $3 );
                      options_d = g_list_copy( alias->options );
                      options = g_list_concat( options_g, options_d );/* Concaténation des listes d'options */
                      switch(alias->bit)
                       { case T_TEMPO  : if ($1)
                                        { char *chaine;
                                          taille = strlen(alias->nom) + strlen(INTERDIT_BARRE_DROITE) + 1;
                                          chaine = New_chaine(taille);
                                          g_snprintf( chaine, taille, INTERDIT_BARRE_DROITE,
                                                      ligne_source_dls, alias->nom );
                                          Emettre_erreur(chaine); g_free(chaine);
                                          erreur++; 

                                          $$=New_action();
                                          taille = 2;
                                          $$->alors = New_chaine( taille );
                                          g_snprintf( $$->alors, taille, " " ); 
                                          $$->sinon = NULL;
                                        }
                                       else $$=New_action_tempo( alias->num, options );
                                       break;
                         case T_MSG  : if ($1)
                                        { char *chaine;
                                          taille = strlen(alias->nom) + strlen(INTERDIT_BARRE_DROITE) + 1;
                                          chaine = New_chaine(taille);
                                          g_snprintf( chaine, taille, INTERDIT_BARRE_DROITE,
                                                      ligne_source_dls, alias->nom );
                                          Emettre_erreur(chaine); g_free(chaine);
                                          erreur++; 

                                          $$=New_action();
                                          taille = 2;
                                          $$->alors = New_chaine( taille );
                                          g_snprintf( $$->alors, taille, " " ); 
                                          $$->sinon = NULL;
                                        }
                                       else $$=New_action_msg( alias->num );
                                       break;
                         case SORTIE : $$=New_action_sortie( alias->num, $1 );       break;
                         case BI     : if (alias->num >= NBR_BIT_BISTABLE_RESERVED)
                                        { $$=New_action_bi( alias->num, $1 ); }
                                       else
                                        { char *chaine;
                                           taille = strlen(INTERDIT_BIT_B_RESERVED) + 1;
                                           chaine = New_chaine(taille);
                                           g_snprintf( chaine, taille, INTERDIT_BIT_B_RESERVED, ligne_source_dls, alias->num );
                                           Emettre_erreur(chaine); g_free(chaine);
                                           erreur++;

                                           $$=New_action();
                                           taille = 2;
                                           $$->alors = New_chaine( taille );
                                           g_snprintf( $$->alors, taille, " " ); 
                                           $$->sinon = NULL;
                                        }
                                       break;
                         case MONO   : if ($1)
                                        { char *chaine;
                                          taille = strlen(alias->nom) + strlen(INTERDIT_BARRE_DROITE) + 2;
                                          chaine = New_chaine(taille);
                                          g_snprintf( chaine, taille, INTERDIT_BARRE_DROITE, ligne_source_dls, alias->nom );
                                          Emettre_erreur(chaine); g_free(chaine);
                                          erreur++;

                                          $$=New_action();
                                          taille = 2;
                                          $$->alors = New_chaine( taille );
                                          g_snprintf( $$->alors, taille, " " ); 
                                          $$->sinon = NULL;
                                        }
                                       else $$=New_action_mono( alias->num );        break;

                         case CPT_H  : $$=New_action_cpt_h( alias->num );            break;
                         case CPT_IMP: $$=New_action_cpt_imp( alias->num, options ); break;
                         case ICONE  : $$=New_action_icone( alias->num, options );   break;
                         default: { char *chaine;
                                    taille = strlen(alias->nom) + strlen(INTERDIT_DROITE) + 1;
                                    chaine = New_chaine(taille);
                                    g_snprintf( chaine, taille, INTERDIT_DROITE, ligne_source_dls, alias->nom );
                                    Emettre_erreur(chaine); g_free(chaine);
                                    erreur++;

                                    $$=New_action();
                                    taille = 2;
                                    $$->alors = New_chaine( taille );
                                    g_snprintf( $$->alors, taille, " " ); 
                                    $$->sinon = NULL;
                                  }
                       }
                      g_list_free(options);
                    }
                   Liberer_options($3);                                /* On libére les options "locales" */
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
ordre:          INF | SUP | INF_OU_EGAL | SUP_OU_EGAL
                ;
/********************************************* Gestion des options ****************************************/
liste_options:  POUV options PFERM   {{ $$ = $2;   }}
                |                    {{ $$ = NULL; }}
                ;

options:        options VIRGULE une_option
                {{ $$ = g_list_append( $1, $3 );
                }}
                | une_option    {{ $$ = g_list_append( NULL, $1 ); }}
                ;

une_option:     MODE EGAL ENTIER
                {{ $$=New_option();
                   $$->type = MODE;
                   $$->entier = $3;
                }}
                | COLOR EGAL couleur
                {{ $$=New_option();
                   $$->type = COLOR;
                   $$->entier = $3;
                }}
                | CLIGNO EGAL ENTIER
                {{ $$=New_option();
                   $$->type = CLIGNO;
                   $$->entier = $3;
                }}
                | CONSIGNE EGAL ENTIER
                {{ $$=New_option();
                   $$->type = CONSIGNE;
                   $$->entier = $3;
                }}
                | RESET EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RESET;
                   $$->entier = $3;
                }}
                | RATIO EGAL ENTIER
                {{ $$=New_option();
                   $$->type = RATIO;
                   $$->entier = $3;
                }}
                ;


couleur:        ROUGE | VERT | BLEU | JAUNE | NOIR | BLANC | GRIS | ORANGE | KAKI
                ;
%%
/**********************************************************************************************************/
/* yyerror: Gestion des erreurs de syntaxe                                                                */
/**********************************************************************************************************/
 int Dls_error ( char *s )
  { char *chaine;
    int taille;

    taille = strlen(ERR_SYNTAXE) + strlen(s) + 5;
    chaine = New_chaine( taille );
    g_snprintf( chaine, taille, ERR_SYNTAXE, ligne_source_dls, s );
    Emettre_erreur( chaine );
    g_free(chaine);
    erreur++;
    return(0);
  }
/**********************************************************************************************************/
/* Interpreter: lecture et production de code intermedaire a partir du fichier en parametre               */
/* Entrée: le nom du fichier originel                                                                     */
/**********************************************************************************************************/
 gboolean Interpreter_source_dls ( gchar *source )
  { gchar chaine[80];
    FILE *rc;

    ligne_source_dls  = 1;                                       /* Initialisation des variables globales */

    erreur=0;
    rc = fopen(source, "r");
    if (rc)
     { Emettre(" #include <Module_dls.h>\n void Go ( int start )\n {\n");
       Dls_debug = 0;                                                        /* Debug de la traduction ?? */
       Dls_restart(rc);
       Dls_parse();
       Emettre(" }\n");

       fclose(rc);
       if (erreur)
        { g_snprintf(chaine, sizeof(chaine), "%d syntax error%s found\n", erreur, (erreur>1 ? "s" : "") );
          Emettre_erreur( chaine );
          return(FALSE);
        } else
        { g_snprintf(chaine, sizeof(chaine), "No syntax error found\n" );
          Emettre_erreur( chaine );
        }
       return(TRUE);
     } else printf("ouverture plugin impossible: niet\n");
    return(FALSE);
  }
/*--------------------------------------------------------------------------------------------------------*/
