/**********************************************************************************************************/
/* Client/Config_cli/Config_cli_mots.l        Définitions des mots de config du client Watchdog           */
/* Projet WatchDog version 1.7       Gestion d'habitat                      ven 03 avr 2009 19:03:13 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_cli_mots.l
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
#include "Config_cli_lignes.h"
 int ligne=1;

%}

digit         [0-9]+
chaine        [a-zA-Z_/][a-zA-Z0-9_/.]+
blanc         [\t ]+

%x comment
%%
{blanc}                    ;

\n                         { ligne++; }
"true"                     { yylval.val = 1;
                             return ENTIER;
                           }
"false"                    { yylval.val = 0;
                             return ENTIER;
                           }
"serveur"                  { yylval.val = SERVEUR;
                             return SERVEUR;
                           }
"user"                     { yylval.val = USER;
                             return USER;
                           }
"SSL_crypt"                { yylval.val = SSL_CRYPT;
                             return SSL_CRYPT;
                           }
"port"                     { yylval.val = PORT;
                             return PORT;
                           }
"taille_bloc_reseau"       { yylval.val = TAILLE_BLOC_RESEAU;
                             return TAILLE_BLOC_RESEAU;
                           }
"debug"                    { yylval.val = DEBUG;
                             return(DEBUG);

                           }
"CONFIG"                   { yylval.val = D_CONFIG;
                             return(D_CONFIG);
                           }
"SIGNAUX"                  { yylval.val = D_SIGNAUX;
                             return(D_SIGNAUX);
                           }
"DB"                       { yylval.val = D_DB;
                             return(D_DB);
                           }
"MEM"                      { yylval.val = D_MEM;
                             return(D_MEM);
                           }
"USER"                     { yylval.val = D_USER;
                             return(D_USER);
                           }
"CRYPTO"                   { yylval.val = D_CRYPTO;
                             return(D_CRYPTO);
                           }
"INFO"                     { yylval.val = D_INFO;
                             return(D_INFO);
                           }
"ALL"                      { yylval.val = D_ALL;
                             return(D_ALL);
                           }
"CDG"                      { yylval.val = D_CDG;
                             return(D_CDG);
                           }
"NETWORK"                  { yylval.val = D_NETWORK;
                             return(D_NETWORK);
                           }
"FORK"                     { yylval.val = D_FORK;
                             return(D_FORK);
                           }
"CONNEXION"                { yylval.val = D_CONNEXION;
                             return(D_CONNEXION);
                           }
"="                        { yylval.val = EGAL;
                             return EGAL;
                           }                        
","                        { yylval.val = VIRGULE;
                             return VIRGULE;
                           }                        
"#"                        { BEGIN(comment); }
<comment>\n                { ligne++; BEGIN(INITIAL); }
<comment>.                 ;

{chaine}                   { yylval.chaine = (char *)strdup(yytext);
                             return CHAINE;
                           }
{digit}                    { yylval.val = atoi(yytext);
                             return ENTIER;
                           }
.                          { return 0; }

%%
