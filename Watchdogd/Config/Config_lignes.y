/**********************************************************************************************************/
/* Watchdogd/Config/Config_lignes.y        Définitions des lignes de config Watchdog                      */
/* Projet WatchDog version 1.6       Gestion d'habitat                      lun 21 avr 2003 20:58:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_lignes.y
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
#include <stdlib.h>

#include "Erreur.h"
#include "Config.h"

extern int ligne;
int erreur;                                                             /* Compteur d'erreur du programme */

%}

%union { int val;
         char *chaine;
       };

%type  <val>     liste_debug one_debug

%token <val>     ENTIER
%token <chaine>  CHAINE

%token   EGAL VIRGULE T_POINT DPOINT TIRET T_AOUV T_AFER T_POUV T_PFER
%token   PORT MAX_CLIENT MAX_MSG_VISU
%token   MIN_SERVEUR MAX_SERVEUR MAX_INACTIVITE
%token   HOME TIMEOUT_CONNEXION MAX_LOGIN_FAILED TAILLE_BLOC_RESEAU
%token   DB_HOST DB_DATABASE DB_PASSWORD DB_USERNAME DB_PORT
%token   PORT_RS485
%token   CRYPTO_KEY TAILLE_CLEF_DH TAILLE_CLEF_RSA
%token   DEBUG D_ALL D_SIGNAUX D_DB D_USER D_CONFIG D_CRYPTO D_INFO D_MEM D_CDG D_NETWORK D_FORK D_MODBUS
%token   D_ADMIN D_CONNEXION D_DLS D_RS485 D_ONDULEUR D_SMS D_AUDIO


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
                { Config.port = $3; }
                | MAX_CLIENT EGAL ENTIER
                { Config.max_client = $3; }
                | MIN_SERVEUR EGAL ENTIER
                { Config.min_serveur = $3; }
                | MAX_SERVEUR EGAL ENTIER
                { Config.max_serveur = $3; }
                | MAX_INACTIVITE EGAL ENTIER
                { Config.max_inactivite = $3; }
                | TAILLE_CLEF_DH EGAL ENTIER
                { Config.taille_clef_dh = $3; }
                | TAILLE_CLEF_RSA EGAL ENTIER
                { Config.taille_clef_rsa = $3; }
                | MAX_MSG_VISU EGAL ENTIER
                { Config.max_msg_visu = $3; }
                | TAILLE_BLOC_RESEAU EGAL ENTIER
                { Config.taille_bloc_reseau = $3; }
                | PORT_RS485 EGAL CHAINE
                { snprintf( Config.port_RS485, TAILLE_PORT_RS485, "%s", $3 );
                  free($3);
                }
                | DB_PORT EGAL ENTIER
                { Config.db_port = $3; }
                | DB_HOST EGAL CHAINE
                { snprintf( Config.db_host, TAILLE_DB_HOST, "%s", $3 );
                  free($3);
                }
                | DB_DATABASE EGAL CHAINE
                { snprintf( Config.db_database, TAILLE_DB_DATABASE, "%s", $3 );
                  free($3);
                }
                | DB_PASSWORD EGAL CHAINE
                { snprintf( Config.db_password, TAILLE_DB_PASSWORD, "%s", $3 );
                  free($3);
                }
                | DB_USERNAME EGAL CHAINE
                { snprintf( Config.db_username, TAILLE_DB_USERNAME, "%s", $3 );
                  free($3);
                }
                | CRYPTO_KEY EGAL CHAINE
                { memcpy( Config.crypto_key, $3, strlen($3) );
                  free($3);
                }
                | MAX_LOGIN_FAILED EGAL ENTIER
                { Config.max_login_failed = $3; }
                | TIMEOUT_CONNEXION EGAL ENTIER
                { Config.timeout_connexion = $3; }
                | HOME EGAL CHAINE
                { snprintf( Config.home, TAILLE_HOME, "%s", $3 );
                  free($3);
                }
                | DEBUG EGAL liste_debug
                { Config.debug_level = $3;
                }
                ;


liste_debug:    one_debug VIRGULE liste_debug
                { $$ = $1 + $3; }
                | one_debug
                { $$ = $1; }
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
                | D_FORK      { $$ = DEBUG_FORK;      }
                | D_CONNEXION { $$ = DEBUG_CONNEXION; }
                | D_DLS       { $$ = DEBUG_DLS;       }
                | D_MODBUS    { $$ = DEBUG_MODBUS;    }
                | D_ADMIN     { $$ = DEBUG_ADMIN;     }
                | D_RS485     { $$ = DEBUG_RS485;     }
                | D_ONDULEUR  { $$ = DEBUG_ONDULEUR;  }
                | D_SMS       { $$ = DEBUG_SMS;       }
                | D_AUDIO     { $$ = DEBUG_AUDIO;     }
                | D_ALL       { $$ = ~0; }
		;
%%
/**********************************************************************************************************/
/* yyerror: Gestion des erreurs de syntaxe                                                                */
/**********************************************************************************************************/
 int yyerror ( char *s )
  { printf( "Erreur syntaxe chargement config ligne %d: %s\n", ligne, s );
    erreur++;
    return(erreur);
  }

/**********************************************************************************************************/
/* Interpreter: Lecture du fichier de config Watchdog dans rc                                             */
/* Entrée: un FILE *rc                                                                                    */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Interpreter_config ( FILE *rc )
  { Config_restart(rc);
    Config_parse();
  }
/*--------------------------------------------------------------------------------------------------------*/
