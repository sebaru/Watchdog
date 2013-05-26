/**********************************************************************************************************/
/* Watchdogd/Utilisateur/clef.c             Recuperation ID et clef utilisateur                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     ven 03 avr 2009 20:31:01 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * clef.c
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
 
 #include <string.h>
 #include <stdlib.h>
 #include <openssl/evp.h>                                                        /* Procedure de cryptage */

/************************************ Prototypes des fonctions ********************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Set_password: Correspond au changement de password de l'utilisateur                                    */
/* Entrées: un log, une db, un id utilisateur, une clef, un password                                      */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Set_password( struct LOG *log, struct DB *db, gchar *clef, struct CMD_UTIL_SETPASSWORD *util )
  { gchar requete[200];
    gchar *crypt, *code_crypt;

    crypt = Crypter( log, clef, util->code_en_clair );
    if (!crypt)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Set_password: cryptage password impossible" );
       return(FALSE);
     }

    code_crypt = Normaliser_chaine ( crypt );
    g_free(crypt);
    if (!code_crypt)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Set_password: Normalisation code crypte impossible" );
       return(FALSE);
     }
 
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET crypt = '%s', changepass=false, login_failed='0' WHERE id=%d",
                NOM_TABLE_UTIL, code_crypt, util->id );
    g_free(code_crypt);

    if ( ! Lancer_requete_SQL ( db, requete ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Set_password: update failed %s", util->id );
       g_free(crypt);
       return(FALSE);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                  "Set_password: update ok for id=%s", util->id );
    g_free(crypt);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Recuperer_clef: essaie de recuperer la clef de l'utilisateur                                           */
/* Entrée: un log, une db, un nom, un entier accessible pour l'id                                         */
/* Sortie: la clef freeable ou null si pb                                                                 */
/**********************************************************************************************************/
 gchar *Recuperer_clef ( struct LOG *log, struct DB *db, gchar *nom, gint *id )
  { gchar requete[200];
    gchar *crypt;

    g_snprintf( requete, sizeof(requete),
                "SELECT id, crypt"
                " FROM %s WHERE name=\'%s\'",
                 NOM_TABLE_UTIL, nom );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recuperer_clef: Key not found in DB for %s", nom );
       return(NULL);
     }

    crypt = (gchar *)g_try_malloc0( NBR_CARAC_CODE_CRYPTE );
    if (!crypt)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_clef: out of memory" );
       Liberer_resultat_SQL (db);
       return(NULL);
     }
   *id = atoi(db->row[0]);                                                      /* Conversion en entier */
    memcpy( crypt, db->row[1], NBR_CARAC_CODE_CRYPTE );

    Liberer_resultat_SQL (db);
    return(crypt);
  }
/**********************************************************************************************************/
/* Crypter: cryptage du password en parametre, selon la clef en parametre et l'algo blowfish              */
/* Entrées: un log, un password, la clef                                                                  */
/* Sortie: la chaine cryptée necessite d'etre frée                                                        */
/**********************************************************************************************************/
 gchar *Crypter( struct LOG *log, gchar *clef, gchar *pass )
  { guchar iv[] = "Watchdog";                                                         /* IV par défaut !! */
    guchar outbuf[NBR_CARAC_CODE_CRYPTE];                                             /* On prévoit large */
    guchar password[NBR_CARAC_LOGIN_UTF8+1];                             /* Pour le formatage du password */
    EVP_CIPHER_CTX ctx;                                                           /* Contexte de cryptage */
    gint outlen, tmplen;
    gchar *result;

    if (!g_utf8_strlen(pass, -1))
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Crypter: password should not be empty" );
       return( NULL );
     }
    if (!strlen(clef))
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Crypter: clef should not be empty" );
       return( NULL );
     }

    memset( password, 0, sizeof(password) );
    memcpy( password, pass, g_utf8_strlen(pass, -1) );

    EVP_CIPHER_CTX_init(&ctx);                                                   /* Initialisation du CTX */
    EVP_EncryptInit(&ctx, EVP_bf_cbc(), password, iv);                           /* Choix de l'algorithme */
                                                          /* Cryptage de la clef par le password, phase 1 */
    if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, clef, strlen(clef)))
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Crypter: Encrypt phase 1 failed" );
       return(NULL);
     }

    if(!EVP_EncryptFinal(&ctx, outbuf + outlen, &tmplen))                            /* Cryptage, phase 2 */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Crypter: Encrypt phase 2 failed" );
       return(NULL);
     }
    outlen += tmplen;
    EVP_CIPHER_CTX_cleanup(&ctx);                                                    /* Cryptage effectué */

    if (outlen>NBR_CARAC_CODE_CRYPTE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Crypter: clef cryptee tronquée (%d)!!", outlen );
       outlen = NBR_CARAC_CODE_CRYPTE;
     } else Info_new( Config.log, Config.log_msrv, LOG_INFO, "Crypter: taille clef cryptee (%d)!!", outlen );

    result = (gchar *)g_try_malloc0( outlen );
    if (!result)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Crypter: memory error" );
       return(NULL);
     }
    memcpy( result, outbuf, outlen );
    return( result );
  }
/*--------------------------------------------------------------------------------------------------------*/
