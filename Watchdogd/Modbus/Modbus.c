/******************************************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                                         */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                     jeu. 24 déc. 2009 12:59:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Modbus.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>
 #include <netinet/in.h>
 #include <netdb.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Modbus.h"

/******************************************************************************************************************************/
/* Modbus_SET_DI_from_master_by_array: Met a jour une DI depuis le master                                                     */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_DI_from_master_by_array ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    struct MODBUS_VARS *vars = module->vars;
    gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *element_thread_tech_id  = Json_get_string ( element, "thread_tech_id" );
    gchar *element_thread_acronyme = Json_get_string ( element, "thread_acronyme" );
    gchar *element_tech_id         = Json_get_string ( element, "tech_id" );
    gchar *element_acronyme        = Json_get_string ( element, "acronyme" );

    if (!element_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque element_thread_tech_id" ); }
    else if (!element_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque element_thread_acronyme" ); }
    else if (strcasecmp (element_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Pas pour nous" ); }
    else if (!Json_has_member ( element, "etat" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque etat" ); }
    else
     { gdouble etat = Json_get_bool ( element, "etat" );
       pthread_mutex_lock ( &module->synchro );
       for (gint num=0; num<vars->nbr_entree_tor; num++)
        { if ( vars->DI && vars->DI[num] &&
               !strcasecmp ( Json_get_string(vars->DI[num], "thread_acronyme"), element_thread_acronyme ) )
           { Json_node_add_bool ( vars->DI[num], "etat", etat );
             Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_DI '%s:%s'/'%s:%s'=%d",
                       element_thread_tech_id, element_thread_acronyme, element_tech_id, element_acronyme, etat );
             break;
           }
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Modbus_SET_DO: Met a jour une sortie TOR en fonction du jsonnode en parametre                                              */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_DO ( struct THREAD *module, JsonNode *msg )
  { struct MODBUS_VARS *vars = module->vars;
    gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "token_lvl1" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "token_lvl2" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque msg_thread_tech_id" ); }
    else if (!msg_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque msg_thread_acronyme" ); }
    else if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Pas pour nous" ); }
    else if (!Json_has_member ( msg, "etat" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque etat" ); }
    else
     { gboolean etat = Json_get_bool ( msg, "etat" );
       pthread_mutex_lock ( &module->synchro );
       for (gint num=0; num<vars->nbr_sortie_tor; num++)
        { if ( vars->DO && vars->DO[num] &&
               !strcasecmp ( Json_get_string(vars->DO[num], "thread_acronyme"), msg_thread_acronyme ) )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_DO '%s:%s'/'%s:%s'=%d",
                       msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, etat );
             Json_node_add_bool ( vars->DO[num], "etat", etat );
             break;
           }
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Modbus_SET_DO_by_array: Initialise les DO recues par le master                                                             */
/* Entrée: les parametres d'une JsonArrayFonction                                                                             */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_DO_from_master_by_array ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    Modbus_SET_DO ( module, element );
  }
/******************************************************************************************************************************/
/* Modbus_SET_AI_from_master_by_array: Met a jour une AI depuis le master                                                     */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_AI_from_master_by_array ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    struct MODBUS_VARS *vars = module->vars;
    gchar *thread_tech_id          = Json_get_string ( module->config, "thread_tech_id" );
    gchar *element_thread_tech_id  = Json_get_string ( element, "thread_tech_id" );
    gchar *element_thread_acronyme = Json_get_string ( element, "thread_acronyme" );
    gchar *element_tech_id         = Json_get_string ( element, "tech_id" );
    gchar *element_acronyme        = Json_get_string ( element, "acronyme" );

    if (!element_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque element_thread_tech_id" ); }
    else if (!element_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque element_thread_acronyme" ); }
    else if (strcasecmp (element_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Pas pour nous" ); }
    else if (!Json_has_member ( element, "valeur" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque etat" ); }
    else if (!Json_has_member ( element, "in_range" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque in_range" ); }
    else
     { gdouble valeur    = Json_get_double ( element, "valeur" );
       gboolean in_range = Json_get_bool   ( element, "in_range" );
       pthread_mutex_lock ( &module->synchro );
       for (gint num=0; num<vars->nbr_entree_ana; num++)
        { if ( vars->AI && vars->AI[num] &&
               !strcasecmp ( Json_get_string(vars->AI[num], "thread_acronyme"), element_thread_acronyme ) )
           { Json_node_add_double ( vars->AI[num], "valeur", valeur );
             Json_node_add_bool   ( vars->AI[num], "in_range", in_range );
             Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_AI '%s:%s'/'%s:%s'=%f (in_range=%d)",
                       element_thread_tech_id, element_thread_acronyme, element_tech_id, element_acronyme, valeur, in_range );
             break;
           }
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Modbus_SET_AO: Met a jour une sortie ANA en fonction du jsonnode en parametre                                              */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_AO ( struct THREAD *module, JsonNode *msg )
  { struct MODBUS_VARS *vars = module->vars;
    gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "token_lvl1" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "token_lvl2" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque msg_thread_tech_id" ); }
    else if (!msg_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "requete mal formée manque msg_thread_acronyme" ); }
    else if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Pas pour nous" ); }
    else if (!Json_has_member ( msg, "valeur" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Requete mal formée manque etat" ); }
    else
     { gdouble valeur = Json_get_double ( msg, "valeur" );
       pthread_mutex_lock ( &module->synchro );
       for (gint num=0; num<vars->nbr_sortie_ana; num++)
        { if ( vars->AO && vars->AO[num] &&
               !strcasecmp ( Json_get_string(vars->AO[num], "thread_acronyme"), msg_thread_acronyme ) )
           { gint type_borne = Json_get_int    ( vars->AO[num], "type_borne" );
             gdouble min     = Json_get_double ( vars->AO[num], "min" );
             gdouble max     = Json_get_double ( vars->AO[num], "max" );
             if (valeur < min) valeur = min;
             if (valeur > max) valeur = max;
             gint new_val_int;
             switch( type_borne )
              { case WAGO_750550: new_val_int = (gint) (4095 * (valeur - min) / max); break;
                default: new_val_int = 0;
              }
             Json_node_add_int ( vars->AO[num], "val_int", new_val_int );
             Info_new( __func__, module->Thread_debug, LOG_NOTICE, "SET_AO '%s:%s'/'%s:%s'=%f (val_int=%d)",
                       msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, valeur, new_val_int );
             break;
           }
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Modbus_SET_AO_by_array: Initialise les AO recues par le master                                                             */
/* Entrée: les parametres d'une JsonArrayFonction                                                                             */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_SET_AO_from_master_by_array ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct THREAD *module = user_data;
    Modbus_SET_AO ( module, element );
  }
/******************************************************************************************************************************/
/* Modbus_Sync_Output_from_master: Synchronise les Output du master vers le wago                                              */
/* Entrée: le module                                                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Modbus_Sync_IO_from_master ( struct THREAD *module )
  { Info_new( __func__, module->Thread_debug, LOG_INFO, "Syncing IO from master" );
    JsonNode *result = Http_Get_from_local_BUS ( module, "GET_IO" );
    if (result)
     { if (Json_has_member ( result, "DI" ) ) Json_node_foreach_array_element ( result, "DI", Modbus_SET_DI_from_master_by_array, module );
       if (Json_has_member ( result, "DO" ) ) Json_node_foreach_array_element ( result, "DO", Modbus_SET_DO_from_master_by_array, module );
       if (Json_has_member ( result, "AI" ) ) Json_node_foreach_array_element ( result, "AI", Modbus_SET_AI_from_master_by_array, module );
       if (Json_has_member ( result, "AO" ) ) Json_node_foreach_array_element ( result, "AO", Modbus_SET_AO_from_master_by_array, module );
     }
    Json_node_unref ( result );
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_module ( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    if (!module) return;
    if (vars->started == FALSE) return;

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    close ( vars->connexion );
    vars->connexion = 0;
    vars->started = FALSE;
    vars->request = FALSE;
    vars->nbr_deconnect++;
    vars->date_retente = Partage->top + MODBUS_RETRY;
    if (vars->DI) { g_free(vars->DI); vars->DI = NULL; }
    if (vars->DO) { g_free(vars->DO); vars->DO = NULL; }
    if (vars->AI) { g_free(vars->AI); vars->AI = NULL; }
    if (vars->AO) { g_free(vars->AO); vars->AO = NULL; }
    vars->nbr_entree_tor = 0;
    vars->nbr_entree_ana = 0;
    vars->nbr_sortie_ana = 0;
    vars->nbr_sortie_tor = 0;
    Thread_send_comm_to_master ( module, FALSE );
    Info_new( __func__, module->Thread_debug, LOG_INFO, "Module '%s' disconnected", hostname );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_module ( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct addrinfo *result, *rp;
    struct timeval sndtimeout;
    struct addrinfo hints;
    gint connexion = 0, s;

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    sndtimeout.tv_sec  = 10;
    sndtimeout.tv_usec =  0;

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Trying to connect module to '%s'", hostname );

    s = getaddrinfo( hostname, "502", &hints, &result);
    if (s != 0)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "getaddrinfo Failed for module %s (%s)", hostname, gai_strerror(s) );
       return(FALSE);
     }

   /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
     { connexion = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
       if (connexion == -1)
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                   "Socket creation failed for modbus '%s'", hostname );
          continue;
        }

       if ( setsockopt ( connexion, SOL_SOCKET, SO_SNDTIMEO, (char *)&sndtimeout, sizeof(sndtimeout)) < 0 )
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                   "Socket Set Options failed for modbus '%s'", hostname );
          continue;
        }

       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( __func__, module->Thread_debug, LOG_INFO,
                   "Using family=%d for host '%s'", rp->ai_family, hostname );

          break;  /* Success */
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                   "'connexion refused by module '%s' family=%d error '%s'",
                   hostname, rp->ai_family, strerror(errno) );
        }
       close(connexion);                                                       /* Suppression de la socket qui n'a pu aboutir */
     }
    freeaddrinfo(result);
    if (rp == NULL) return(FALSE);                                                                     /* Erreur de connexion */

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    vars->connexion = connexion;                                                                          /* Sauvegarde du fd */
    vars->date_last_reponse = Partage->top;
    vars->date_retente   = 0;
    vars->transaction_id = 1;
    vars->started        = TRUE;
    vars->mode           = MODBUS_GET_DESCRIPTION;
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Module Connected" );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_description( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2020 );
    requete.nbr            = htons( 16 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                      /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_firmware( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2023 );
    requete.nbr            = htons( 16 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Init_watchdog1( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );                                                                   /* Stop Timer */
    requete.valeur         = htons( 0x0000 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog2( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1009 );                                   /* Close MODBUS socket after watchdog timeout */
    requete.valeur         = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: This register stores the watchdog timeout value as an unsigned 16 bit value. The Description default value is 0.   */
/* Setting this value will not trigger the watchdog. However, a non zero value must be stored in this register before the     */
/* watchdog can be triggered. The time value is stored in multiples of 100ms (e.g., 0x0009 is .9 seconds). It is not possible */
/* to modify this value while the watchdog is running                                                                         */
/******************************************************************************************************************************/
 static void Init_watchdog3( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1000 );                                                       /* Watchdog Time register */
    requete.valeur         = htons( Json_get_int ( module->config, "watchdog" ) ); /* coupure sortie, en 100ième de secondes  */

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog4( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );
    requete.valeur         = htons( 0x0001 );                                                                  /* Start Timer */

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre d'entree ANALOGIQUE                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_ANA( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1023 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre de sortie ANALOGIQUE                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_ANA( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1022 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_TOR : Demander au module d'envoyer son nombre d'entree TOR                                           */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_TOR( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1025 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
               "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_sortie_TOR : Demander au module d'envoyer son nombre de sortie TOR                                          */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_TOR( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1024 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "OK" );
       vars->request = TRUE;                                                                      /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_tor( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_entree_tor );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_entree_ana: Interrogation des entrees analogique d'un module wago                                               */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_ana( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_entree_ana );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_sortie_tor( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_poid, cpt_byte, cpt, taille, nbr_data;

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    nbr_data = ((vars->nbr_sortie_tor-1)/8)+1;
    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0007 + nbr_data;
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_sortie_tor );                                                    /* bit count */
    requete.data[2]        = nbr_data;                                                                          /* Byte count */

    if (vars->DO)
     { for ( cpt_poid = 1, cpt_byte = 3, cpt = 0; cpt<vars->nbr_sortie_tor; cpt++ )
        { if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
          if ( vars->DO[cpt] )
           { if (Json_get_bool ( vars->DO[cpt], "etat" )) { requete.data[cpt_byte] |= cpt_poid; } }
          cpt_poid = cpt_poid << 1;
        }
     }

    gint retour = write ( vars->connexion, &requete, taille+6 );
    if ( retour != taille+6 )                                                                          /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_sortie_ana: Envoie les informations liées aux sorties ANA du module                                             */
/* Entrée: le module à interroger                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Interroger_sortie_ana( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_byte, cpt, taille;

    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0006 + (vars->nbr_sortie_ana*2 + 1);
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_sortie_ana );                                                      /* bit count */
    requete.data[2]        = (vars->nbr_sortie_ana*2);                                                          /* Byte count */

    if (vars->AO)
     { for ( cpt_byte = 3, cpt = 0; cpt<vars->nbr_sortie_ana; cpt++)
        { if (vars->AO[cpt])
           { gint val_int = Json_get_int ( vars->AO[cpt], "val_int" );
             requete.data [cpt_byte  ] =  val_int >> 5;
             requete.data [cpt_byte+1] = (val_int & 0x1F)<<3;
             cpt_byte += 2;
           }
        }
     }

    gint retour = write ( vars->connexion, &requete, taille+6 );
    if ( retour != taille+6 )                                                                          /* Envoi de la requete */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "Failed for module '%s': error %d/%s", hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Modbus_load_io_config : Charge les données des IO du module                                                                */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Modbus_load_io_config ( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;

/***************************************************** Mapping des AnalogInput ************************************************/
    Info_new( __func__, module->Thread_debug, LOG_INFO, "Allocate %d AI", vars->nbr_entree_ana );
    if(vars->nbr_entree_ana)
     { vars->AI = g_try_malloc0( sizeof(JsonNode *) * vars->nbr_entree_ana );
       if (vars->AI)
        { JsonArray *array = Json_get_array ( module->config, "AI" );
          for ( gint cpt = 0; cpt < json_array_get_length ( array ); cpt++ )
           { JsonNode *element = json_array_get_element ( array, cpt );
             gint num = Json_get_int ( element, "num" );
             if ( 0 <= num && num < vars->nbr_entree_ana )
              { vars->AI[num] = element;
                Json_node_add_double ( vars->AI[num], "valeur", 0.0 );
                Json_node_add_bool   ( vars->AI[num], "in_range", FALSE );
                Info_new( __func__, module->Thread_debug, LOG_NOTICE, "New AI '%s' (%s, %s)",
                          Json_get_string ( vars->AI[num], "thread_acronyme" ),
                          Json_get_string ( vars->AI[num], "libelle" ),
                          Json_get_string ( vars->AI[num], "unite" ) );
              } else Info_new( __func__, module->Thread_debug, LOG_WARNING, "Map AI: num %d out of range '%d'",
                               num, vars->nbr_entree_ana );
           }
        }
       else Info_new( __func__, module->Thread_debug, LOG_ERR, "Memory Error for AI" );
     }
/***************************************************** Mapping des DigitalInput ***********************************************/
    Info_new( __func__, module->Thread_debug, LOG_INFO, "Allocate %d DI", vars->nbr_entree_tor );
    if(vars->nbr_entree_tor)
     { vars->DI = g_try_malloc0( sizeof(JsonNode *) * vars->nbr_entree_tor );
       if (vars->DI)
        { JsonArray *array = Json_get_array ( module->config, "DI" );
          for ( gint cpt = 0; cpt < json_array_get_length ( array ); cpt++ )
           { JsonNode *element = json_array_get_element ( array, cpt );
             gint num = Json_get_int ( element, "num" );
             if ( 0 <= num && num < vars->nbr_entree_tor )
              { vars->DI[num] = element;
                Json_node_add_bool ( vars->DI[num], "etat", FALSE );
                Info_new( __func__, module->Thread_debug, LOG_NOTICE, "New DI '%s' (%s), flip=%d",
                          Json_get_string ( vars->DI[num], "thread_acronyme" ),
                          Json_get_string ( vars->DI[num], "libelle" ),
                          Json_get_bool   ( vars->DI[num], "flip" ));
              } else Info_new( __func__, module->Thread_debug, LOG_WARNING, "Map DI: num %d out of range '%d'",
                                num, vars->nbr_entree_tor );
           }
        }
       else Info_new( __func__, module->Thread_debug, LOG_ERR, "Memory Error for DI" );
     }
/***************************************************** Mapping des AnalogOutput ***********************************************/
    Info_new( __func__, module->Thread_debug, LOG_INFO, "Allocate %d AO", vars->nbr_sortie_ana );
    if(vars->nbr_sortie_ana)
     { vars->AO = g_try_malloc0( sizeof(JsonNode *) * vars->nbr_sortie_ana );
       if (vars->AO)
        { JsonArray *array = Json_get_array ( module->config, "AO" );
          for ( gint cpt = 0; cpt < json_array_get_length ( array ); cpt++ )
           { JsonNode *element = json_array_get_element ( array, cpt );
             gint num = Json_get_int ( element, "num" );
             if ( 0 <= num && num < vars->nbr_sortie_ana )
              { vars->AO[num] = element;
                Json_node_add_double ( vars->AO[num], "valeur", 0.0 );
                Json_node_add_int    ( vars->AO[num], "val_int", 0 );
                Info_new( __func__, module->Thread_debug, LOG_NOTICE, "New AO '%s' (%s, %s)",
                          Json_get_string ( vars->AO[num], "thread_acronyme" ),
                          Json_get_string ( vars->AI[num], "libelle" ),
                          Json_get_string ( vars->AI[num], "unite" ) );
              } else Info_new( __func__, module->Thread_debug, LOG_WARNING, "map AO: num %d out of range '%d'",
                               num, vars->nbr_sortie_ana );
           }
        }
       else Info_new( __func__, module->Thread_debug, LOG_ERR, "Memory Error for AO" );
     }
/***************************************************** Mapping des DigitalOutput **********************************************/
    Info_new( __func__, module->Thread_debug, LOG_INFO, "Allocate %d DO", vars->nbr_sortie_tor );
    if(vars->nbr_sortie_tor)
     { vars->DO = g_try_malloc0( sizeof(JsonNode *) * vars->nbr_sortie_tor );
       if (vars->DO)
        { JsonArray *array = Json_get_array ( module->config, "DO" );
          for ( gint cpt = 0; cpt < json_array_get_length ( array ); cpt++ )
           { JsonNode *element = json_array_get_element ( array, cpt );
             gint num = Json_get_int ( element, "num" );
             if ( 0 <= num && num < vars->nbr_sortie_tor )
              { vars->DO[num] = element;
                Json_node_add_bool   ( vars->DO[num], "etat", FALSE );
                Info_new( __func__, module->Thread_debug, LOG_NOTICE, "New DO '%s' (%s)",
                          Json_get_string ( vars->DO[num], "thread_acronyme" ),
                          Json_get_string ( vars->DO[num], "libelle" ));
              } else Info_new( __func__, module->Thread_debug, LOG_WARNING, "map DO: num %d out of range '%d'",
                               num, vars->nbr_sortie_tor );
           }
        }
       else Info_new( __func__, module->Thread_debug, LOG_ERR, " Memory Error for DO" );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    Modbus_Sync_IO_from_master( module );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Module '%s' : io config done",
              Json_get_string ( module->config, "description" ) );
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Modbus_Processer_trame( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    vars->nbr_oct_lu = 0;
    vars->request = FALSE;                                                                       /* Une requete a été traitée */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if ( (guint16) vars->response.proto_id )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING, "Wrong proto_id" );
       Deconnecter_module( module );
     }

    gint cpt_byte, cpt_poid, cpt;
    vars->date_last_reponse = Partage->top;                                                        /* Estampillage de la date */
    Thread_send_comm_to_master ( module, TRUE );
    if (ntohs(vars->response.transaction_id) != vars->transaction_id)                                     /* Mauvaise reponse */
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Wrong transaction_id: attendu %d, recu %d",
                 vars->transaction_id, ntohs(vars->response.transaction_id) );
     }
    if ( vars->response.fct >=0x80 )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Erreur Reponse, Error %d, Exception code %d",
                 vars->response.fct, (int)vars->response.data[0] );
       Deconnecter_module( module );
       return;
     }
    switch (vars->mode)
     { case MODBUS_GET_DI:
            for ( cpt_poid = 1, cpt_byte = 1, cpt = 0; cpt<vars->nbr_entree_tor; cpt++)
             { if (vars->DI[cpt])                                                                   /* Si l'entrée est mappée */
                { gint new_etat_int = (vars->response.data[ cpt_byte ] & cpt_poid);
                  gboolean new_etat = (new_etat_int ? TRUE : FALSE);
                  if ( Json_get_bool ( vars->DI[cpt], "flip" ) ) new_etat = new_etat ^ 1;
                  MQTT_Send_DI ( module, vars->DI[cpt], new_etat );
                }
               cpt_poid = cpt_poid << 1;
               if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
             }
            vars->mode = MODBUS_GET_AI;
            break;
       case MODBUS_GET_AI:
            for ( cpt = 0; cpt<vars->nbr_entree_ana; cpt++)
             { if (vars->AI[cpt])                                                                   /* Si l'entrée est mappée */
                { gint type_borne = Json_get_int ( vars->AI[cpt], "type_borne" );
                  gboolean new_in_range;
                  gdouble new_valeur;
                  switch( type_borne )
                   { case WAGO_750455:                                                                       /* Borne 4/20 mA */
                      { gint16 new_valeur_int  = (gint16)vars->response.data[ 2*cpt + 1 ] << 5;
                               new_valeur_int |= (gint16)vars->response.data[ 2*cpt + 2 ] >> 3;
                        gdouble min  = Json_get_double ( vars->AI[cpt], "min" );
                        gdouble max  = Json_get_double ( vars->AI[cpt], "max" );
                        new_valeur   = ((gint)new_valeur_int*(max - min))/4095.0 + min;
                        new_in_range = !(vars->response.data[ 2*cpt + 2 ] & 0x03);
                        break;
                      }
                     case WAGO_750461:                                                                         /* Borne PT100 */
                      { gint16 new_valeur_int  = (gint16)vars->response.data[ 2*cpt + 1 ] << 8;
                               new_valeur_int |= (gint16)vars->response.data[ 2*cpt + 2 ];
                        new_valeur  = ((gint)new_valeur_int)/10.0;
                        if (new_valeur_int > -2000 && new_valeur_int < 8500) new_in_range = TRUE; else new_in_range = FALSE;
                        break;
                      }
                     default : new_valeur=0.0; new_in_range=FALSE;
                   }
                  MQTT_Send_AI ( module, vars->AI[cpt], new_valeur, new_in_range );
                }
             }
            vars->mode = MODBUS_SET_DO;
            break;
       case MODBUS_SET_DO:
            vars->mode = MODBUS_SET_AO;
            break;
       case MODBUS_SET_AO:
            vars->mode = MODBUS_GET_DI;
            break;
       case MODBUS_GET_DESCRIPTION:
          { gchar chaine[32];
            gint taille;
            memset ( chaine, 0, sizeof(chaine) );
            taille = vars->response.data[0];
            if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
            chaine[0] = ntohs( (gint16)vars->response.data[1] );
            chaine[2] = ntohs( (gint16)vars->response.data[3] );
            chaine[taille] = 0;
            Info_new( __func__, module->Thread_debug, LOG_INFO, "Description (size %d) = '%s'", taille, chaine );
            vars->mode = MODBUS_GET_FIRMWARE;
            break;
         }
       case MODBUS_GET_FIRMWARE:
          { gchar chaine[64];
            gint taille;
            memset ( chaine, 0, sizeof(chaine) );
            taille = vars->response.data[0];
            if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
            chaine[0] = ntohs( (gint16)vars->response.data[1] );
            chaine[2] = ntohs( (gint16)vars->response.data[3] );
            chaine[taille] = 0;
            Info_new( __func__, module->Thread_debug, LOG_INFO, "Firmware (size %d) = '%s'", taille, chaine );
            vars->mode = MODBUS_INIT_WATCHDOG1;
            break;
         }
       case MODBUS_INIT_WATCHDOG1:
            Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Watchdog1 = %d %d",
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG2;
            break;
       case MODBUS_INIT_WATCHDOG2:
            Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Watchdog2 = %d %d",
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG3;
            break;
       case MODBUS_INIT_WATCHDOG3:
            Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Watchdog3 = %d %d",
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG4;
            break;
       case MODBUS_INIT_WATCHDOG4:
            Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Watchdog4 = %d %d",
                     ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_GET_NBR_AI;
            break;
       case MODBUS_GET_NBR_AI:
             { vars->nbr_entree_ana = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) ) / 16;
               Info_new( __func__, module->Thread_debug, LOG_INFO, "Get %03d Entree ANA", vars->nbr_entree_ana );
               vars->mode = MODBUS_GET_NBR_AO;
             }
            break;
       case MODBUS_GET_NBR_AO:
             { vars->nbr_sortie_ana = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) ) / 16;
               Info_new( __func__, module->Thread_debug, LOG_INFO, "Get %03d Sortie ANA", vars->nbr_sortie_ana );
               vars->mode = MODBUS_GET_NBR_DI;
             }
            break;
       case MODBUS_GET_NBR_DI:
             { gint nbr;
               nbr = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) );
               vars->nbr_entree_tor = nbr;
               Info_new( __func__, module->Thread_debug, LOG_INFO, "Get %03d Entree TOR", vars->nbr_entree_tor );
               vars->mode = MODBUS_GET_NBR_DO;
             }
            break;
       case MODBUS_GET_NBR_DO:
             { vars->nbr_sortie_tor = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) );
               Info_new( __func__, module->Thread_debug, LOG_INFO, "Get %03d Sortie TOR", vars->nbr_sortie_tor );
               Modbus_load_io_config( module );                                                  /* Initialise les IO modules */
               JsonNode *RootNode = Json_node_create ();                                          /* Envoi de la conf a l'API */
               if (!RootNode) break;
               Json_node_add_string ( RootNode, "thread_tech_id", thread_tech_id );
               Json_node_add_int    ( RootNode, "nbr_entree_tor", vars->nbr_entree_tor );
               Json_node_add_int    ( RootNode, "nbr_entree_ana", vars->nbr_entree_ana );
               Json_node_add_int    ( RootNode, "nbr_sortie_tor", vars->nbr_sortie_tor );
               Json_node_add_int    ( RootNode, "nbr_sortie_ana", vars->nbr_sortie_ana );
               JsonNode *API_result = Http_Post_to_global_API ( "/run/modbus/add/io", RootNode );
               Json_node_unref ( API_result );
               Json_node_unref ( RootNode );
               vars->mode = MODBUS_GET_DI;
             }
            break;
     }
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Recuperer_reponse_module( struct THREAD *module )
  { struct MODBUS_VARS *vars = module->vars;
    fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    if (vars->date_last_reponse + 600 < Partage->top)                                      /* Detection attente trop longue */
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "Timeout module started=%d, mode=%02d, "
                "transactionID=%06d, nbr_deconnect=%02d, last_reponse=%03ds ago, retente=in %03ds, date_next_eana=in %03ds",
                 vars->started, vars->mode, vars->transaction_id, vars->nbr_deconnect,
                (Partage->top - vars->date_last_reponse)/10,
                (vars->date_retente > Partage->top   ? (vars->date_retente   - Partage->top)/10 : -1),
                (vars->date_next_eana > Partage->top ? (vars->date_next_eana - Partage->top)/10 : -1)
               );
       Deconnecter_module( module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(vars->connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 1000;                                                                               /* Attente d'un caractere */
    retval = select(vars->connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>0 && FD_ISSET(vars->connexion, &fdselect) )
     { int bute;
       if (vars->nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(vars->response.taille); }

       if (bute>=sizeof(struct TRAME_MODBUS_REPONSE))
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                   "bute = %d >= %d (sizeof(module->reponse)=%d, taille recue = %d)",
                    bute, sizeof(struct TRAME_MODBUS_REPONSE), sizeof(vars->response), ntohs(vars->response.taille) );
          Deconnecter_module( module );
          return;
        }

       cpt = read( vars->connexion, (unsigned char *)&vars->response + vars->nbr_oct_lu, bute-vars->nbr_oct_lu );
       if (cpt>=0)
        { vars->nbr_oct_lu += cpt;
          if (vars->nbr_oct_lu >= TAILLE_ENTETE_MODBUS + ntohs(vars->response.taille))
           { Modbus_Processer_trame( module ); }                                    /* Si l'on a trouvé une trame complète !! */
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_WARNING, "Read Error. Get %d, error %s", cpt, strerror(errno) );
          Deconnecter_module ( module );
        }
      }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct MODBUS_VARS) );
    struct MODBUS_VARS *vars = module->vars;

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          if (Json_has_member ( request, "token_lvl0" ))
           { gchar *token_lvl0 = Json_get_string ( request, "token_lvl0" );
                  if ( !strcasecmp (token_lvl0, "SET_DO") )  Modbus_SET_DO ( module, request );
             else if ( !strcasecmp (token_lvl0, "SET_AO") )  Modbus_SET_AO ( module, request );
             else if ( !strcasecmp (token_lvl0, "SYNC_IO") ) Modbus_Sync_IO_from_master ( module );
           }
          Json_node_unref ( request );
        }
/********************************************* Début de l'interrogation du module *********************************************/
       if ( vars->started == FALSE )                                               /* Si attente retente, on change de module */
        { if ( vars->date_retente <= Partage->top && Connecter_module(module)==FALSE )
           { Info_new( __func__, module->Thread_debug, LOG_INFO, "Module DOWN. retrying in %ds", MODBUS_RETRY/10 );
             vars->date_retente = Partage->top + MODBUS_RETRY;
           }
        }
       else for (gint i=0; i<4; i++)                                     /* 1 tour programme = 4 itérations GET (DI/DO/AI/AO) */
        { if ( vars->request )                                                           /* Requete en cours pour ce module ? */
           { Recuperer_reponse_module ( module ); }
          else
           { if (vars->date_next_eana<Partage->top)                                    /* Gestion décalée des I/O Analogiques */
              { vars->date_next_eana = Partage->top + MBUS_TEMPS_UPDATE_IO_ANA;                        /* Tous les 5 dixiemes */
                vars->do_check_eana = TRUE;
              }
             switch (vars->mode)
              { case MODBUS_GET_DESCRIPTION: Interroger_description( module ); break;
                case MODBUS_GET_FIRMWARE   : Interroger_firmware( module ); break;
                case MODBUS_INIT_WATCHDOG1 : Init_watchdog1( module ); break;
                case MODBUS_INIT_WATCHDOG2 : Init_watchdog2( module ); break;
                case MODBUS_INIT_WATCHDOG3 : Init_watchdog3( module ); break;
                case MODBUS_INIT_WATCHDOG4 : Init_watchdog4( module ); break;
                case MODBUS_GET_NBR_AI     : Interroger_nbr_entree_ANA( module ); break;
                case MODBUS_GET_NBR_AO     : Interroger_nbr_sortie_ANA( module ); break;
                case MODBUS_GET_NBR_DI     : Interroger_nbr_entree_TOR( module ); break;
                case MODBUS_GET_NBR_DO     : Interroger_nbr_sortie_TOR( module ); break;
                case MODBUS_GET_DI         : if (vars->nbr_entree_tor) Interroger_entree_tor( module );
                                             else vars->mode = MODBUS_GET_AI;
                                             break;
                case MODBUS_GET_AI         : if (vars->nbr_entree_ana && vars->do_check_eana)
                                              { Interroger_entree_ana( module ); }
                                             else vars->mode = MODBUS_SET_DO;
                                             break;
                case MODBUS_SET_DO         : if (vars->nbr_sortie_tor) Interroger_sortie_tor( module );
                                             else vars->mode = MODBUS_SET_AO;
                                             break;
                case MODBUS_SET_AO         : if (vars->nbr_sortie_ana && vars->do_check_eana)
                                              { Interroger_sortie_ana( module ); }
                                             else vars->mode = MODBUS_GET_DI;
                                             vars->do_check_eana = FALSE;                                /* Le check est fait */
                                             break;
              }
           }
        }
     }
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
