/**********************************************************************************************************/
/* Client/Config_cli_cli/Config_cli_lignes.y     Définitions des lignes de config du client Watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:02:58 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_cli_lignes.y
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
#include <stdlib.h>

#include "Erreur.h"
#include "Config_cli.h"

extern int ligne;
int erreur;                                                             /* Compteur d'erreur du programme */
static struct CONFIG_CLI *Config_cli;
%}

%union { int val;
         char *chaine;
       }

%type  <val>     liste_debug one_debug
%token <val>     ENTIER
%token <chaine>  CHAINE

%token   EGAL VIRGULE
%token   SERVEUR PORT SSL_CRYPT USER
%token   TAILLE_BLOC_RESEAU
%token   DEBUG D_ALL D_SIGNAUX D_DB D_USER D_CONFIG D_CRYPTO D_INFO D_MEM D_CDG D_NETWORK
%token   D_CONNEXION


%%
fichier:        lignes_ou_vide;

lignes_ou_vide: lignes
                |
                ;

/******************************************* Gestion des instructions *************************************/
lignes:         une_ligne lignes
                | une_ligne
                ;

une_ligne:      PORT EGAL ENTIER
                { Config_cli->port = $3; }
                | SSL_CRYPT EGAL ENTIER
                { Config_cli->ssl_crypt = ($3 ? 1 : 0); }
                | TAILLE_BLOC_RESEAU EGAL ENTIER
                { Config_cli->taille_bloc_reseau = $3; }
                | DEBUG EGAL liste_debug
                { Config_cli->debug_level = $3;
                }
                | SERVEUR EGAL CHAINE
                { g_snprintf( Config_cli->serveur, sizeof(Config_cli->serveur), "%s", $3 );
                  g_free($3);
                }
                | USER EGAL CHAINE
                { g_snprintf( Config_cli->user, sizeof(Config_cli->user), "%s", $3 );
                  g_free($3);
                }
                ;

liste_debug:	one_debug
                { $$ = $1; }
                | one_debug VIRGULE liste_debug
                { $$ = $1 + $3; }
		;

one_debug:
		D_SIGNAUX     { $$ = DEBUG_SIGNAUX;   }
                | D_DB        { $$ = DEBUG_DB;        }
                | D_CONFIG    { $$ = DEBUG_CONFIG;    }
                | D_USER      { $$ = DEBUG_USER;      }
                | D_CRYPTO    { $$ = DEBUG_CRYPTO;    }
                | D_INFO      { $$ = DEBUG_INFO;      }
                | D_MEM       { $$ = DEBUG_MEM;       }
                | D_CDG       { $$ = DEBUG_CDG;       }
                | D_NETWORK   { $$ = DEBUG_NETWORK;   }
                | D_CONNEXION { $$ = DEBUG_CONNEXION; }
                | D_ALL       { $$ = ~0; }
		;
%%
/**********************************************************************************************************/
/* yyerror: Gestion des erreurs de syntaxe                                                                */
/**********************************************************************************************************/
 int yyerror ( char *s )
  { printf( "Erreur syntaxe chargement config ligne %d\n", ligne );
    erreur++;
    return(erreur);
  }

/**********************************************************************************************************/
/* Interpreter: Lecture du fichier de config Watchdog dans rc                                             */
/* Entrée: un FILE *rc                                                                                    */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Interpreter ( FILE *rc, struct CONFIG_CLI *config )
  { if (!config) return;
    Config_cli = config;

    yyrestart(rc);
    yyparse();
  }
/*--------------------------------------------------------------------------------------------------------*/
