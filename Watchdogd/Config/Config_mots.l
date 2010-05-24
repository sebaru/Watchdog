/**********************************************************************************************************/
/* Watchdogd/Config/Config_mots.l      Définitions des mots de config Watchdog                            */
/* Projet WatchDog version 1.7       Gestion d'habitat                      lun 21 avr 2003 20:59:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Config_mots.l
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
#include "Config_lignes.h"
 int ligne=1;

%}

 /* On n'utilise pas la fonction Config_wrap associée */
%option noyywrap

 /* Expressions rationnelles de detection */
digit         [0-9]+
chaine        [a-zA-Z_/][a-zA-Z0-9._/]+
blanc         [\t ]+

 /* Noms des environnements */
%x comment
%%
{blanc}                    ;

\n                         { ligne++; }
"true"                     { Config_lval.val = 1;
                             return ENTIER;
                           }
"false"                    { Config_lval.val = 0;
                             return ENTIER;
                           }
"port_rs485"               { Config_lval.val = PORT_RS485;
                             return PORT_RS485;
                           }
"home_dir"                 { Config_lval.val = HOME;
                             return HOME;
                           }
"taille_clef_dh"           { Config_lval.val = TAILLE_CLEF_DH;
                             return TAILLE_CLEF_DH;
                           }
"taille_clef_rsa"          { Config_lval.val = TAILLE_CLEF_RSA;
                             return TAILLE_CLEF_RSA;
                           }
"db_host"                  { Config_lval.val = DB_HOST;
                             return DB_HOST;
                           }
"db_port"                  { Config_lval.val = DB_PORT;
                             return DB_PORT;
                           }
"db_database"              { Config_lval.val = DB_DATABASE;
                             return DB_DATABASE;
                           }
"db_password"              { Config_lval.val = DB_PASSWORD;
                             return DB_PASSWORD;
                           }
"db_username"              { Config_lval.val = DB_USERNAME;
                             return DB_USERNAME;
                           }
"crypto_key"               { Config_lval.val = CRYPTO_KEY;
                             return CRYPTO_KEY;
                           }
"max_client"               { Config_lval.val = MAX_CLIENT;
                             return MAX_CLIENT;
                           }
"min_serveur"              { Config_lval.val = MIN_SERVEUR;
                             return MIN_SERVEUR;
                           }
"max_serveur"              { Config_lval.val = MAX_SERVEUR;
                             return MAX_SERVEUR;
                           }
"max_inactivite"           { Config_lval.val = MAX_INACTIVITE;
                             return MAX_INACTIVITE;
                           }
"max_msg_visu"             { Config_lval.val = MAX_MSG_VISU;
                             return MAX_MSG_VISU;
                           }
"max_login_failed"         { Config_lval.val = MAX_LOGIN_FAILED;
                             return MAX_LOGIN_FAILED;
                           }
"port"                     { Config_lval.val = PORT;
                             return PORT;
                           }
"timeout_connexion"        { Config_lval.val = TIMEOUT_CONNEXION;
                             return TIMEOUT_CONNEXION;
                           }
"taille_bloc_reseau"       { Config_lval.val = TAILLE_BLOC_RESEAU;
                             return TAILLE_BLOC_RESEAU;
                           }
"debug"                    { Config_lval.val = DEBUG;
                             return(DEBUG);
                           }
"CONFIG"                   { Config_lval.val = D_CONFIG;
                             return(D_CONFIG);
                           }
"SIGNAUX"                  { Config_lval.val = D_SIGNAUX;
                             return(D_SIGNAUX);
                           }
"DB"                       { Config_lval.val = D_DB;
                             return(D_DB);
                           }
"MEM"                      { Config_lval.val = D_MEM;
                             return(D_MEM);
                           }
"USER"                     { Config_lval.val = D_USER;
                             return(D_USER);
                           }
"CRYPTO"                   { Config_lval.val = D_CRYPTO;
                             return(D_CRYPTO);
                           }
"INFO"                     { Config_lval.val = D_INFO;
                             return(D_INFO);
                           }
"DLS"                      { Config_lval.val = D_DLS;
                             return(D_DLS);
                           }
"ALL"                      { Config_lval.val = D_ALL;
                             return(D_ALL);
                           }
"CDG"                      { Config_lval.val = D_CDG;
                             return(D_CDG);
                           }
"NETWORK"                  { Config_lval.val = D_NETWORK;
                             return(D_NETWORK);
                           }
"FORK"                     { Config_lval.val = D_FORK;
                             return(D_FORK);
                           }
"MODBUS"                   { Config_lval.val = D_MODBUS;
                             return(D_MODBUS);
                           }
"ADMIN"                    { Config_lval.val = D_ADMIN;
                             return(D_ADMIN);
                           }
"RS485"                    { Config_lval.val = D_RS485;
                             return(D_RS485);
                           }
"ONDULEUR"                 { Config_lval.val = D_ONDULEUR;
                             return(D_ONDULEUR);
                           }
"CONNEXION"                { Config_lval.val = D_CONNEXION;
                             return(D_CONNEXION);
                           }
"="                        { Config_lval.val = EGAL;
                             return EGAL;
                           }                        
","                        { Config_lval.val = VIRGULE;
                             return VIRGULE;
                           }                        
":"                        { Config_lval.val = DPOINT;
                             return DPOINT;
                           }                        
"."                        { Config_lval.val = T_POINT;
                             return T_POINT;
                           }                        
"-"                        { Config_lval.val = TIRET;
                             return TIRET;
                           }                        
"{"                        { Config_lval.val = T_AOUV;
                             return T_AOUV;
                           }                        
"}"                        { Config_lval.val = T_AFER;
                             return T_AFER;
                           }                        
"("                        { Config_lval.val = T_POUV;
                             return T_POUV;
                           }                        
")"                        { Config_lval.val = T_PFER;
                             return T_PFER;
                           }                        
"#"                        { BEGIN(comment); }
<comment>\n                { ligne++; BEGIN(INITIAL); }
<comment>.                 ;

{chaine}                   { Config_lval.chaine = (char *)strdup(Config_text);
                             return CHAINE;
                           }
{digit}                    { Config_lval.val = atoi(Config_text);
                             return ENTIER;
                           }
.                          { return 0; }

%%
