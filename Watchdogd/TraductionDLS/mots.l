/**********************************************************************************************************/
/* Watchdogd/TraductionDLS/mots.l        Définitions des mots DLS                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 avr 2009 19:53:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * mots.l
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
 
%{
#include <stdio.h>
#include <string.h>
#include "Proto_traductionDLS.h"
#include "lignes.h"
 int ligne_source_dls=1;
%}

 /* On n'utilise pas la fonction yywrap associée */
%option noyywrap

 /* Expressions rationnelles de detection */
digit         [0-9]+
digitf        [-]?[0-9]+[.[0-9]+]?
id            [a-zA-Z][a-zA-Z_0-9]+
blanc         [\t ]+

 /* Noms des environnements */
%x code_en_c
%x comment
%%
{blanc}                    ;

"\n"                       { ligne_source_dls++; }

 /******************************************** Methodes ***************************************************/
"_E"                       { printf("digit %s\n", Dls_text);
                             return ENTREE;      }
"_EA"                      { printf("digit %s\n", Dls_text);
                             return EANA;        }
"_A"                       { printf("digit %s\n", Dls_text);
                             return SORTIE;      }
"_I"                       { printf("digit %s\n", Dls_text);
                             return ICONE;       }
"_B"                       { printf("digit %s\n", Dls_text);
                             return BI;          }
"_M"                       { printf("digit %s\n", Dls_text);
                             return MONO;        }
"_TR"                      { printf("digit %s\n", Dls_text);
                             return TEMPO;       }
"_CH"                      { printf("digit %s\n", Dls_text);
                             return CPT_H;       }
"_MSG"                     { printf("digit %s\n", Dls_text);
                             return MSG;         }
"_START"                   { printf("digit %s\n", Dls_text);
                             return START;       }

 /***************************************** Methodes speciales ********************************************/
"HEURE"                    { return HEURE;       }
"APRES"                    { return APRES;       }
"AVANT"                    { return AVANT;       }

 /************************************************ Couleurs ***********************************************/
"rouge"                    { return ROUGE;       }
"vert"                     { return VERT;        }
"bleu"                     { return BLEU;        }
"jaune"                    { return JAUNE;       }
"noir"                     { return NOIR;        }
"orange"                   { return ORANGE;      }
"gris"                     { return GRIS;        }
"blanc"                    { return BLANC;       }
 /************************************************* Options ************************************************/
"mode"                     { return MODE;        }
"consigne"                 { return CONSIGNE;    }
"color"                    { return COLOR;       }
"cligno"                   { return CLIGNO;      }
"slave"                    { return SLAVE;       }
 /*********************************************** Le reste ************************************************/
"."                        { printf("digit %s\n", Dls_text);
                             return ET;          }
"+"                        { printf("digit %s\n", Dls_text);
                             return OU;          }
"/"                        { printf("digit %s\n", Dls_text);
                             return BARRE;       }
"-"                        { printf("digit %s\n", Dls_text);
                             return MOINS;       }
";"                        { printf("digit %s\n", Dls_text);
                             return PVIRGULE;    }
","                        { printf("digit %s\n", Dls_text);
                             return VIRGULE;     }
"("                        { printf("digit %s\n", Dls_text);
                             return POUV;        }
")"                        { printf("digit %s\n", Dls_text);
                             return PFERM;       }
"="                        { printf("digit %s\n", Dls_text);
                             return EGAL;        }
"<->"                      { printf("digit %s\n", Dls_text);
                             return EQUIV;       }
"->"                       { printf("digit %s\n", Dls_text);
                             return DONNE;       }
"<"                        { printf("digit %s\n", Dls_text);
                             return INF;         }
">"                        { printf("digit %s\n", Dls_text);
                             return SUP;         }
"<="                       { printf("digit %s\n", Dls_text);
                             return INF_OU_EGAL; }
">="                       { printf("digit %s\n", Dls_text);
                             return SUP_OU_EGAL; }

"CodeC"                    { BEGIN(code_en_c); }
<code_en_c>\n              { BEGIN(INITIAL); Emettre(Dls_text); ligne_source_dls++; }
<code_en_c>.               { Emettre(Dls_text); }

"/*"                       { BEGIN(comment); }
<comment>"*/"              { BEGIN(INITIAL); }
<comment>\n                { ligne_source_dls++; }
<comment>.                 ;

{id}                       { printf("ID %s\n", Dls_text);
                             Dls_lval.chaine = (char *)strdup(Dls_text);
                             return ID;
                           }
{digit}                    { printf("digit %s\n", Dls_text);
                             Dls_lval.val = atoi(Dls_text);
                             return ENTIER;
                           }
{digitf}                   { printf("digitf %s\n", Dls_text);
                             Dls_lval.valf = atof(Dls_text);
                             return VALF;
                           }
.                          { printf(". %s\n", Dls_text);
                              }
%%
