/**********************************************************************************************************/
/* Watchdogd/initdb.c        Création de la base de données Watchdog v2.0                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 26 jan 2004 16:33:08 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>
 #include <string.h>

 #include "Erreur.h"
 #include "Utilisateur_DB.h"
 #include "Synoptiques_DB.h"
 #include "Mnemonique_DB.h"
 #include "Message_DB.h"
 #include "Histo_DB.h"
 #include "Dls_DB.h"
 #include "Icones_DB.h"
 #include "EntreeANA_DB.h"
 #include "ValANA_DB.h"
 #include "Archive_DB.h"
 #include "Cpth_DB.h"
 #include "Config.h"

/* extern struct DB *Db_watchdog;                                                      /* Database Watchdog */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */

/**********************************************************************************************************/
/* Concat: concatenation de 2 chaines en gérant les free necessaires                                      */
/* Entrée: 2 chaines (gchar *)  on suppose que l'on doit freer chaine 1 et que chaine 2 ne le doit pas    */
/* Sortie: une chaine concaténée                                                                          */
/**********************************************************************************************************/
 static gchar *Concat( gchar *chaine1, gchar *chaine2 )
  { gchar *result;
    if (chaine1)
     { result = g_strconcat( chaine1, chaine2, NULL );
       g_free(chaine1);
     }
    else result = g_strdup(chaine2);
    return(result);
  }

/**********************************************************************************************************/
/* initDb_watchdog: initialisation des tables de la Db_watchdog watchdog                                  */
/* entrées: un log et une database                                                                        */
/* sortie: Un pointeur sur une chaine de caractere si probleme, null sinon                                */
/**********************************************************************************************************/
 gchar *Init_db_watchdog( void )
  { gchar *chaine_globale, *old;
    struct DB *Db_watchdog;
    gchar chaine[2048];
    struct LOG *log;

    Db_watchdog = ConnexionDB( Config.log, Config.db_name,            /* Oui: connexion en tant que Admin */
                               Config.db_admin_username, Config.db_password );
    if (!Db_watchdog)
     { g_snprintf( chaine, sizeof(chaine), _("Init_db_watchdog: Failed to open dsn %s, user %s\n"),
                   Config.db_name, Config.db_admin_username );
       return( strdup(chaine) );
     }
       
    log = Config.log;
    memset( chaine, 0, sizeof(chaine) );
    old = chaine_globale = NULL;                                               /* Par défaut, pas de bobo */

/*********************************** Gestion des utilisateurs Watchdog ************************************/
    if (! Creer_db_gids( log, Db_watchdog ))
                                                            /* Si pb lors de la creation de la table gids */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_GIDS, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }

    if (! Creer_db_groupe( log, Db_watchdog ))            /* Si pb lors de la creation de la table groups */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_GROUPE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }

    if (! Creer_db_util( log, Config.crypto_key, Db_watchdog ))
                                                           /* Si pb lors de la creation de la table users */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_UTIL, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/******************************** Gestion des messages au fil de l'eau ************************************/
    if (! Creer_db_msg( log, Db_watchdog ))                  /* Si pb lors de la creation de la table msg */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_MSG, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }

/***************************************** Gestion des plugins DLS ****************************************/
    if (! Creer_db_dls( log, Db_watchdog ))                  /* Si pb lors de la creation de la table msg */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_DLS, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/***************************************** Gestion des historiques ****************************************/
    if (! Creer_db_histo( log, Db_watchdog ))                /* Si pb lors de la creation de la table msg */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_HISTO, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
    if (! Creer_db_histo_hard( log, Db_watchdog ))           /* Si pb lors de la creation de la table msg */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_HISTO_HARD, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/***************************************** Gestion des classes ********************************************/
    if (! Creer_db_classe( log, Db_watchdog ))            /* Si pb lors de la creation de la table classe */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_CLASSE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/***************************************** Gestion des icones *********************************************/
    if (! Creer_db_icone( log, Db_watchdog ))              /* Si pb lors de la creation de la table icone */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_ICONE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/***************************************** Gestion des synoptiques ****************************************/
    if (! Creer_db_synoptique( log, Db_watchdog ))    /* Si pb lors de la creation de la table synoptique */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_SYNOPTIQUE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/***************************************** Gestion des synoptiques ****************************************/
    if (! Creer_db_mnemo( log, Db_watchdog ))         /* Si pb lors de la creation de la table mnemonique */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_MNEMO, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des motifs *****************************************/
    if (! Creer_db_motif( log, Db_watchdog ))              /* Si pb lors de la creation de la table motif */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_MOTIF, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des commentaires ***********************************/
    if (! Creer_db_comment ( log, Db_watchdog ))         /* Si pb lors de la creation de la table comment */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_COMMENT, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des passerelles ************************************/
    if (! Creer_db_passerelle ( log, Db_watchdog ))         /* Si pb lors de la creation de la table pass */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_PASSERELLE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des passerelles ************************************/
    if (! Creer_db_palette ( log, Db_watchdog ))            /* Si pb lors de la creation de la table pass */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_PALETTE, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des motifs *****************************************/
    if (! Creer_db_capteur( log, Db_watchdog ))              /* Si pb lors de la creation de la table motif */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_CAPTEUR, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des entreeANA **************************************/
    if (! Creer_db_entreeANA ( log, Db_watchdog ))          /* Si pb lors de la creation de la table eana */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_ENTREEANA, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des valANA *****************************************/
    if (! Creer_db_valANA ( log, Db_watchdog ))           /* Si pb lors de la creation de la table valana */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_VALANA, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des valANA *****************************************/
    if (! Creer_db_cpth ( log, Db_watchdog ))               /* Si pb lors de la creation de la table cpth */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_CPTH, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/********************************************* Gestion des valANA *****************************************/
    if (! Creer_db_arch ( log, Db_watchdog ))               /* Si pb lors de la creation de la table arch */
     { g_snprintf( chaine, sizeof(chaine), _("Trouble for %s database: %s\n\n"),
                   NOM_TABLE_ARCH, Db_watchdog->last_err );
       chaine_globale = Concat ( chaine_globale, chaine );
     }
/*********************************** Message d'erreur final ***********************************************/
    DeconnexionDB( log, &Db_watchdog );                                         /* Deconnexion de la base */
    return(chaine_globale);
  } 
/*--------------------------------------------------------------------------------------------------------*/
