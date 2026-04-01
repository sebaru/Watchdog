/******************************************************************************************************************************/
/* Watchdogd/sms.c        Gestion des SMS de Watchdog v2.0                                                                    */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                   ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Sms.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <gio/gio.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sms.h"

/******************************************************************************************************************************/
/* ModemManager D-Bus constants                                                                                              */
/******************************************************************************************************************************/
 #define MM_DBUS_SERVICE "org.freedesktop.ModemManager1"
 #define MM_DBUS_ROOT_OBJECT "/org/freedesktop/ModemManager1"
 #define MM_DBUS_OBJ_MANAGER_IFACE "org.freedesktop.DBus.ObjectManager"
 #define MM_DBUS_PROPERTIES_IFACE "org.freedesktop.DBus.Properties"
 #define MM_DBUS_MODEM_IFACE "org.freedesktop.ModemManager1.Modem"
 #define MM_DBUS_MESSAGING_IFACE "org.freedesktop.ModemManager1.Modem.Messaging"
 #define MM_DBUS_SMS_IFACE "org.freedesktop.ModemManager1.Sms"

/******************************************************************************************************************************/
/* Smsg_get_modem_path: Cherche un modem avec interface Messaging via ObjectManager                                          */
/******************************************************************************************************************************/
 static gboolean Smsg_get_modem_path ( struct THREAD *module, gchar **modem_path )
  { struct SMS_VARS *vars = module->vars;
    GDBusConnection *system_bus;
    GVariant *reply;
    GVariantIter *objects;
    const gchar *object_path;
    GVariant *interfaces;
    GError *error = NULL;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vars->mm_modem_path)
     { *modem_path = g_strdup(vars->mm_modem_path);
       return(TRUE);
     }

    system_bus = g_bus_get_sync ( G_BUS_TYPE_SYSTEM, NULL, &error );
    if (!system_bus)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Cannot connect to system D-Bus (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       return(FALSE);
     }

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          MM_DBUS_ROOT_OBJECT,
                                          MM_DBUS_OBJ_MANAGER_IFACE,
                                          "GetManagedObjects",
                                          NULL,
                                          G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );

    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: GetManagedObjects failed (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_object_unref(system_bus);
       return(FALSE);
     }

    g_variant_get ( reply, "(a{oa{sa{sv}}})", &objects );
    while ( g_variant_iter_next ( objects, "{&o@a{sa{sv}}}", &object_path, &interfaces ) )
     { GVariant *messaging = g_variant_lookup_value ( interfaces, MM_DBUS_MESSAGING_IFACE, NULL );
       if (messaging)
        { vars->mm_modem_path = g_strdup(object_path);
          *modem_path = g_strdup(object_path);
          g_variant_unref(messaging);
          g_variant_unref(interfaces);
          break;
        }
       g_variant_unref(interfaces);
     }

    g_variant_iter_free(objects);
    g_variant_unref(reply);
    g_object_unref(system_bus);

    if (*modem_path == NULL)
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: No ModemManager modem with Messaging interface found", thread_tech_id );
       return(FALSE);
     }
    return(TRUE);
  }

/******************************************************************************************************************************/
/* Smsg_get_signal_quality: Lit la qualité du signal modem via D-Bus                                                         */
/******************************************************************************************************************************/
 static gboolean Smsg_get_signal_quality ( struct THREAD *module, gdouble *signal_quality )
  { gchar *modem_path = NULL;
    GDBusConnection *system_bus;
    GVariant *reply;
    GVariant *value;
    GError *error = NULL;
    guint32 quality = 0;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (!Smsg_get_modem_path(module, &modem_path)) return(FALSE);

    system_bus = g_bus_get_sync ( G_BUS_TYPE_SYSTEM, NULL, &error );
    if (!system_bus)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Cannot connect to system D-Bus (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_free(modem_path);
       return(FALSE);
     }

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          modem_path,
                                          MM_DBUS_PROPERTIES_IFACE,
                                          "Get",
                                          g_variant_new ( "(ss)", MM_DBUS_MODEM_IFACE, "SignalQuality" ),
                                          G_VARIANT_TYPE("(v)"),
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );

    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Cannot read signal quality (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_object_unref(system_bus);
       g_free(modem_path);
       return(FALSE);
     }

    g_variant_get ( reply, "(v)", &value );
    if ( g_variant_is_of_type(value, G_VARIANT_TYPE("(ub)")) )
     { gboolean recent;
       g_variant_get ( value, "(ub)", &quality, &recent );
     }
    else if ( g_variant_is_of_type(value, G_VARIANT_TYPE_UINT32) )
     { quality = g_variant_get_uint32(value); }
    else
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Unsupported SignalQuality type '%s'", thread_tech_id, g_variant_get_type_string(value) );
       g_variant_unref(value);
       g_variant_unref(reply);
       g_object_unref(system_bus);
       g_free(modem_path);
       return(FALSE);
     }

    *signal_quality = quality;
    g_variant_unref(value);
    g_variant_unref(reply);
    g_object_unref(system_bus);
    g_free(modem_path);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_gsm: Envoi un sms par le gsm                                                                                     */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Envoi_sms_gsm ( struct THREAD *module, JsonNode *msg, gchar *telephone )
  { struct SMS_VARS *vars = module->vars;
    GDBusConnection *system_bus;
    gchar *modem_path = NULL;
    gchar *sms_path = NULL;
    GVariantBuilder builder;
    GVariant *reply;
    gchar libelle[256];
    GError *error = NULL;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (!telephone)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: telephone is NULL", thread_tech_id );
       return(FALSE);
     }

    if (!Smsg_get_modem_path(module, &modem_path))
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: No modem available, cannot send SMS to '%s'", thread_tech_id, telephone );
       return(FALSE);
     }

    gchar *dls_shortname = Json_get_string ( msg, "dls_shortname" );
    if (dls_shortname) g_snprintf( libelle, sizeof(libelle), "%s: %s", dls_shortname, Json_get_string( msg, "libelle") );
                  else g_snprintf( libelle, sizeof(libelle), "%s", Json_get_string( msg, "libelle") );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
              "%s: Try to send to %s (%s)", thread_tech_id, telephone, libelle );

    system_bus = g_bus_get_sync ( G_BUS_TYPE_SYSTEM, NULL, &error );
    if (!system_bus)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Cannot connect to system D-Bus (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_free(modem_path);
       return(FALSE);
     }

    g_variant_builder_init ( &builder, G_VARIANT_TYPE("a{sv}") );
    g_variant_builder_add ( &builder, "{sv}", "number", g_variant_new_string(telephone) );
    g_variant_builder_add ( &builder, "{sv}", "text",   g_variant_new_string(libelle) );

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          modem_path,
                                          MM_DBUS_MESSAGING_IFACE,
                                          "Create",
                                          g_variant_new ( "(a{sv})", &builder ),
                                          G_VARIANT_TYPE("(o)"),
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );

    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: ModemManager Create failed (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_object_unref(system_bus);
       g_free(modem_path);
       return(FALSE);
     }

    const gchar *sms_path_tmp;
    g_variant_get ( reply, "(&o)", &sms_path_tmp );
    sms_path = g_strdup(sms_path_tmp);
    g_variant_unref(reply);

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          sms_path,
                                          MM_DBUS_SMS_IFACE,
                                          "Send",
                                          NULL,
                                          NULL,
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );

    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Envoi SMS Nok to %s (%s) -> %s", thread_tech_id, telephone, libelle,
                 error ? error->message : "unknown" );
       g_clear_error(&error);
       g_object_unref(system_bus);
       g_free(modem_path);
       g_free(sms_path);
       return(FALSE);
     }
    g_variant_unref(reply);

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          modem_path,
                                          MM_DBUS_MESSAGING_IFACE,
                                          "Delete",
                                          g_variant_new ( "(o)", sms_path ),
                                          NULL,
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          NULL );
    if (reply) g_variant_unref(reply);

    Info_new( __func__, module->Thread_debug, LOG_NOTICE,
              "%s: Envoi SMS Ok to %s (%s)", thread_tech_id, telephone, libelle );
    vars->nbr_sms++;
    g_object_unref(system_bus);
    g_free(modem_path);
    g_free(sms_path);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par OVH                                                                                     */
/* Entrée: le message à envoyer à l'utilisateur                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoi_sms_ovh ( struct THREAD *module, JsonNode *msg, gchar *telephone )
  { gchar clair[512], hash_string[48], signature[48], query[128];
    unsigned char hash_bin[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx;
    int md_len;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    JsonNode *RootNode = Json_node_create();
    Json_node_add_bool  ( RootNode, "noStopClause", TRUE );
    Json_node_add_string( RootNode, "priority", "high" );
    Json_node_add_bool  ( RootNode, "senderForResponse", TRUE );
    Json_node_add_int   ( RootNode, "validityPeriod", 2880 ); /* 2 jours */
    Json_node_add_string( RootNode, "charset", "UTF-8" );

    JsonArray *receivers = Json_node_add_array ( RootNode, "receivers" );
    Json_array_add_element ( receivers, json_node_init_string ( json_node_alloc(), telephone ) );

    gchar libelle[256];
    g_snprintf( libelle, sizeof(libelle), "%s: %s", Json_get_string ( msg, "dls_shortname" ), Json_get_string( msg, "libelle") );
    Json_node_add_string( RootNode, "message", libelle );

    gchar *method = "POST";
    g_snprintf( query, sizeof(query), "https://eu.api.ovh.com/1.0/sms/%s/jobs", Json_get_string ( module->config, "ovh_service_name" ) );
    gchar timestamp[20];
    g_snprintf( timestamp, sizeof(timestamp), "%ld", time(NULL) );

/******************************************************* Calcul signature *****************************************************/
    gchar *body = Json_node_to_string( RootNode );
    g_snprintf( clair, sizeof(clair), "%s+%s+%s+%s+%s+%s",
                Json_get_string ( module->config, "ovh_application_secret" ),
                Json_get_string ( module->config, "ovh_consumer_key" ),
                method, query, body, timestamp );
    Info_new ( __func__, module->Thread_debug, LOG_DEBUG, "Sending to OVH : %s", body );
    g_free(body);

    mdctx = EVP_MD_CTX_new();                                                                               /* Calcul du SHA1 */
    EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
    EVP_DigestUpdate(mdctx, clair, strlen(clair));
    EVP_DigestFinal_ex(mdctx, hash_bin, &md_len);
    EVP_MD_CTX_free(mdctx);

    memset( hash_string, 0, sizeof(hash_string));                                                       /* Conversion en Hexa */
    for (gint i=0; i<20; i++)
     { gchar chaine[3];
       g_snprintf(chaine, sizeof(chaine), "%02x", hash_bin[i] );
       g_strlcat(hash_string, chaine, sizeof(hash_string) );
     }

    g_snprintf( signature, sizeof(signature), "$1$%s", hash_string );

/********************************************************* Préparation des headers ********************************************/
    gchar header[256];
    GSList *liste = NULL;
    g_snprintf ( header, sizeof(header), "X-Ovh-Application: %s", Json_get_string ( module->config, "ovh_application_key" ) );
    liste = g_slist_append ( liste, g_strdup(header) );
    g_snprintf ( header, sizeof(header), "X-Ovh-Consumer: %s",  Json_get_string ( module->config, "ovh_consumer_key" ) );
    liste = g_slist_append ( liste, g_strdup(header) );
    g_snprintf ( header, sizeof(header), "X-Ovh-Signature: %s", signature );
    liste = g_slist_append ( liste, g_strdup(header) );
    g_snprintf ( header, sizeof(header), "X-Ovh-Timestamp: %s", timestamp );
    liste = g_slist_append ( liste, g_strdup(header) );

/********************************************************* Envoi de la requete ************************************************/
    JsonNode *response = Http_Request ( query, RootNode, liste );
    gint http_code = Json_get_int ( response, "http_code" );
    g_slist_free_full ( liste, g_free );
    Json_node_unref ( RootNode );

    if (http_code!=200)
     { /*gchar *reason_phrase = soup_message_get_reason_phrase ( soup_msg );*/
       Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Status %d", thread_tech_id, http_code );
     }
    else Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: '%s' sent to '%s'", thread_tech_id, libelle, telephone );
    Json_node_unref ( response );
  }
/******************************************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                                                  */
/* Entrée: le message à envoyer sateur                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoi_sms_freeapi ( struct THREAD *module, JsonNode *msg, JsonNode *user )
  { gchar libelle_utf8[512];
    g_snprintf( libelle_utf8, sizeof(libelle_utf8), "%s: %s", Json_get_string ( msg, "dls_shortname" ), Json_get_string( msg, "libelle") );
    gchar *libelle = g_uri_escape_string(libelle_utf8, NULL, FALSE);
    if (libelle == NULL)
     { Info_new( __func__,module->Thread_debug, LOG_ERR, "Convert error for %s. Not sending message.", libelle_utf8 );
       return;
     }

    gchar target_uri[512];
    g_snprintf ( target_uri, sizeof(target_uri), "https://smsapi.free-mobile.fr/sendmsg?user=%s&pass=%s&msg=%s",
                 Json_get_string ( user, "free_sms_api_user" ), Json_get_string ( user, "free_sms_api_key" ), libelle
               );
    g_free(libelle);

/********************************************************* Envoi de la requete ************************************************/
    JsonNode *response = Http_Request ( target_uri, NULL, NULL );
    gint http_code = Json_get_int ( response, "http_code" );
    Json_node_unref ( response );

    if (http_code!=200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "Status %d for '%s' to '%s'",
                 http_code, libelle_utf8, Json_get_string ( user, "email" ) );
     }
    else Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s' sent to '%s'", libelle_utf8, Json_get_string ( user, "email" ) );
  }
/******************************************************************************************************************************/
/* Smsg_send_to_all_authorized_recipients : Envoi à tous les portables autorisés                                              */
/* Entrée: le message                                                                                                         */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Smsg_send_to_all_authorized_recipients ( struct THREAD *module, JsonNode *msg )
  { struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vars->sending_is_disabled == TRUE)                                   /* Si envoi désactivé, on sort de suite de la fonction */
     { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending is disabled. Dropping message", thread_tech_id );
       return;
     }

/********************************************* Chargement des informations en bases *******************************************/
    JsonNode *UsersNode = Http_Get_from_global_API ( "/run/users/wanna_be_notified", NULL );
    if (!UsersNode || Json_get_int ( UsersNode, "http_code" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USERS from API", thread_tech_id );
       return;
     }

    gint notif_sms = Json_get_int ( msg, "notif_sms" );
    if (notif_sms == TXT_NOTIF_BY_DLS) { notif_sms = Json_get_int ( msg, "notif_sms_by_dls" ); }

    GList *Recipients = json_array_get_elements ( Json_get_array ( UsersNode, "recipients" ) );
    GList *recipients = Recipients;
    while(recipients)
     { JsonNode *user = recipients->data;
       gchar *user_phone = Json_get_string ( user, "phone" );
       if (!user_phone)
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                    "%s: Warning: User %s does not have an Phone number", thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else if (!strlen(user_phone))
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                    "%s: Warning: User %s has an empty Phone number", thread_tech_id, Json_get_string ( user, "email" ) );
        }
       else switch (notif_sms)
        { case TXT_NOTIF_YES:
               if ( Envoi_sms_gsm ( module, msg, user_phone ) == FALSE )
                { Info_new( __func__, module->Thread_debug, LOG_ERR, "Error sending with GSM" );
                  gchar *free_sms_api_user = Json_get_string ( user, "free_sms_api_user" );
                  if (free_sms_api_user && strlen(free_sms_api_user))
                   { Info_new( __func__, module->Thread_debug, LOG_INFO, "Sending with FREE API" );
                     Envoi_sms_freeapi( module, msg, user );
                   }
                  else
                   { Info_new( __func__, module->Thread_debug, LOG_INFO, "Sending with OVH" );
                     Envoi_sms_ovh( module, msg, user_phone );
                   }
                }
               break;
          case TXT_NOTIF_OVH_ONLY:
               Envoi_sms_ovh ( module, msg, user_phone );
               break;
        }
       recipients = g_list_next(recipients);
     }
    g_list_free(Recipients);
    Json_node_unref ( UsersNode );
    MQTT_Send_AI ( module, vars->ai_nbr_sms, vars->nbr_sms, TRUE );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_ovh_text ( struct THREAD *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode) return;
    Json_node_add_string ( RootNode, "token_lvl0", "SEND_SMS" );
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "notif_sms", TXT_NOTIF_OVH_ONLY );
    pthread_mutex_lock ( &module->synchro );                                                 /* on passe le message au thread */
    module->MQTT_messages = g_slist_append ( module->MQTT_messages, RootNode );
    pthread_mutex_unlock ( &module->synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                                                  */
/* Entrée: un texte au format UTF8 si possible                                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_smsg_gsm_text ( struct THREAD *module, gchar *texte )
  { JsonNode *RootNode = Json_node_create();
    if (!RootNode) return;
    Json_node_add_string ( RootNode, "token_lvl0", "SEND_SMS" );
    Json_node_add_string ( RootNode, "libelle", texte );
    Json_node_add_string ( RootNode, "dls_shortname", Json_get_string ( module->config, "thread_tech_id" ) );
    Json_node_add_int    ( RootNode, "notif_sms", TXT_NOTIF_YES );
    pthread_mutex_lock ( &module->synchro );                                                 /* on passe le message au thread */
    module->MQTT_messages = g_slist_append ( module->MQTT_messages, RootNode );
    pthread_mutex_unlock ( &module->synchro );
  }
/******************************************************************************************************************************/
/* Valeurs de state SMS ModemManager                                                                                          */
/******************************************************************************************************************************/
 #define MM_SMS_STATE_RECEIVED 3

/******************************************************************************************************************************/
/* Traiter_commande_sms: Fonction appelée pour traiter la commande sms recu par le telephone                                  */
/* Entrée: le message text à traiter                                                                                          */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Traiter_commande_sms ( struct THREAD *module, gchar *from, gchar *texte )
  { struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    JsonNode *RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Memory Error for '%s'", thread_tech_id, from );
       return;
     }
    Json_node_add_string ( RootNode, "phone", from );

    JsonNode *UserNode = Http_Post_to_global_API ( "/run/user/can_send_txt_cde", RootNode );
    Json_node_unref ( RootNode );
    if (!UserNode || Json_get_int ( UserNode, "http_code" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USER from API for '%s'", thread_tech_id, from );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "email" ) )
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: %s is not an known user. Dropping command '%s'...", thread_tech_id, from, texte );
       goto end_user;
     }

    if ( !Json_has_member ( UserNode, "can_send_txt_cde" ) || Json_get_bool ( UserNode, "can_send_txt_cde" ) == FALSE )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: %s ('%s') is not allowed to send txt_cde. Dropping command '%s'...", thread_tech_id,
                from, Json_get_string ( UserNode, "email" ), texte );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "ping" ) )                                                               /* Interfacage de test */
     { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Ping Received from '%s'. Sending Pong", thread_tech_id, from );
       Envoyer_smsg_gsm_text ( module, "Pong !" );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "smsoff" ) )                                                                      /* Smspanic ! */
     { vars->sending_is_disabled = TRUE;
       Envoyer_smsg_gsm_text ( module, "Sending SMS is off !" );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending SMS is DISABLED by '%s'", thread_tech_id, from );
       goto end_user;
     }

    if ( ! strcasecmp( texte, "smson" ) )                                                                       /* Smspanic ! */
     { Envoyer_smsg_gsm_text ( module, "Sending SMS is on !" );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending SMS is ENABLED by '%s'", thread_tech_id, from );
       vars->sending_is_disabled = FALSE;
       goto end_user;
     }

    RootNode = Json_node_create();
    if ( RootNode == NULL )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: MapNode Error for '%s'", thread_tech_id, from );
       goto end_user;
     }
    Json_node_add_string ( RootNode, "thread_tech_id", "_COMMAND_TEXT" );
    Json_node_add_string ( RootNode, "thread_acronyme", texte );

    JsonNode *MapNode = Http_Post_to_global_API ( "/run/mapping/search_txt", RootNode );
    Json_node_unref ( RootNode );
    if (!MapNode || Json_get_int ( MapNode, "http_code" ) != 200)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "%s: Could not get USER from API for '%s'", thread_tech_id, from );
       goto end_map;
     }

    if ( Json_has_member ( MapNode, "nbr_results" ) == FALSE )
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': Error searching Database for '%s'", thread_tech_id, texte );
       Envoyer_smsg_gsm_text ( module, "Error searching Database .. Sorry .." );
       goto end_map;
     }

    gint nbr_results = Json_get_int ( MapNode, "nbr_results" );
    if ( nbr_results == 0 )
     { Envoyer_smsg_gsm_text ( module, "Je n'ai pas trouvé, désolé." ); }
    else
     { if ( nbr_results > 1 )                                               /* Si trop d'enregistrements, demande de préciser */
        { Envoyer_smsg_gsm_text ( module, "Aîe, plusieurs choix sont possibles ... :" ); }

       GList *Results = json_array_get_elements ( Json_get_array ( MapNode, "results" ) );
       if ( nbr_results > 1 )
        { GList *results = Results;
          while(results)
           { JsonNode *element = results->data;
             gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
             gchar *tech_id         = Json_get_string ( element, "tech_id" );
             gchar *acronyme        = Json_get_string ( element, "acronyme" );
             gchar *libelle         = Json_get_string ( element, "libelle" );
             Info_new( __func__, module->Thread_debug, LOG_INFO, "'%s': From '%s' map found for '%s' -> '%s:%s' - %s",
                       thread_tech_id, from, thread_acronyme, tech_id, acronyme, libelle );
             Envoyer_smsg_gsm_text ( module, thread_acronyme );                                 /* Envoi des différents choix */
             results = g_list_next(results);
           }
        }
       else if ( nbr_results == 1)
        { JsonNode *element = Results->data;
          gchar *thread_acronyme = Json_get_string ( element, "thread_acronyme" );
          gchar *tech_id         = Json_get_string ( element, "tech_id" );
          gchar *acronyme        = Json_get_string ( element, "acronyme" );
          gchar *libelle         = Json_get_string ( element, "libelle" );
          Info_new( __func__, module->Thread_debug, LOG_INFO, "'%s': From '%s' map found for '%s' (%s)-> '%s:%s' - %s",
                    thread_tech_id, from, Json_get_string( UserNode, "email" ), thread_acronyme, tech_id, acronyme, libelle );
          MQTT_Send_DI_pulse ( module, tech_id, acronyme );
          gchar chaine[256];
          g_snprintf ( chaine, sizeof(chaine), "'%s' fait.", texte );
          Envoyer_smsg_gsm_text ( module, chaine );
        }
       g_list_free(Results);
     }
end_map:
  if (MapNode) Json_node_unref ( MapNode );
end_user:
  if (UserNode) Json_node_unref ( UserNode );
  }

/******************************************************************************************************************************/
/* Smsg_sms_get_property: Lit une propriété d'un objet SMS ModemManager                                                      */
/******************************************************************************************************************************/
 static GVariant *Smsg_sms_get_property ( struct THREAD *module, GDBusConnection *system_bus, gchar *sms_path, gchar *property )
  { GError *error = NULL;
    GVariant *reply;
    GVariant *value;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          sms_path,
                                          MM_DBUS_PROPERTIES_IFACE,
                                          "Get",
                                          g_variant_new ( "(ss)", MM_DBUS_SMS_IFACE, property ),
                                          G_VARIANT_TYPE("(v)"),
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );
    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Cannot read SMS property '%s' (%s)", thread_tech_id, property, error ? error->message : "unknown" );
       g_clear_error(&error);
       return(NULL);
     }
    g_variant_get ( reply, "(v)", &value );
    g_variant_unref(reply);
    return(value);
  }

/******************************************************************************************************************************/
/* Lire_sms_gsm: Lecture des SMS entrants via ModemManager D-Bus                                                             */
/******************************************************************************************************************************/
 static gboolean Lire_sms_gsm ( struct THREAD *module )
  { GDBusConnection *system_bus;
    gchar *modem_path = NULL;
    GError *error = NULL;
    GVariant *reply;
    GVariantIter *sms_iter;
    const gchar *sms_path;
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (!Smsg_get_modem_path(module, &modem_path)) return(FALSE);

    system_bus = g_bus_get_sync ( G_BUS_TYPE_SYSTEM, NULL, &error );
    if (!system_bus)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "%s: Cannot connect to system D-Bus (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_free(modem_path);
       return(FALSE);
     }

    reply = g_dbus_connection_call_sync ( system_bus,
                                          MM_DBUS_SERVICE,
                                          modem_path,
                                          MM_DBUS_MESSAGING_IFACE,
                                          "List",
                                          NULL,
                                          G_VARIANT_TYPE("(ao)"),
                                          G_DBUS_CALL_FLAGS_NONE,
                                          10000,
                                          NULL,
                                          &error );
    if (!reply)
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Cannot list SMS (%s)", thread_tech_id, error ? error->message : "unknown" );
       g_clear_error(&error);
       g_object_unref(system_bus);
       g_free(modem_path);
       return(FALSE);
     }

    g_variant_get ( reply, "(ao)", &sms_iter );
    while ( g_variant_iter_next ( sms_iter, "&o", &sms_path ) )
     { GVariant *state_v = Smsg_sms_get_property ( module, system_bus, (gchar *)sms_path, "State" );
       if (!state_v) continue;
       guint32 state = g_variant_get_uint32(state_v);
       g_variant_unref(state_v);

       if (state == MM_SMS_STATE_RECEIVED)
        { GVariant *number_v = Smsg_sms_get_property ( module, system_bus, (gchar *)sms_path, "Number" );
          GVariant *text_v   = Smsg_sms_get_property ( module, system_bus, (gchar *)sms_path, "Text" );
          if (number_v && text_v)
           { const gchar *from = g_variant_get_string(number_v, NULL);
             const gchar *texte = g_variant_get_string(text_v, NULL);
             Info_new( __func__, module->Thread_debug, LOG_NOTICE,
                      "%s: Recu '%s' from '%s' via %s", thread_tech_id, texte, from, sms_path );
             Traiter_commande_sms ( module, (gchar *)from, (gchar *)texte );
           }
          if (number_v) g_variant_unref(number_v);
          if (text_v) g_variant_unref(text_v);

          GVariant *delete_reply = g_dbus_connection_call_sync ( system_bus,
                                                                  MM_DBUS_SERVICE,
                                                                  modem_path,
                                                                  MM_DBUS_MESSAGING_IFACE,
                                                                  "Delete",
                                                                  g_variant_new ( "(o)", sms_path ),
                                                                  NULL,
                                                                  G_DBUS_CALL_FLAGS_NONE,
                                                                  10000,
                                                                  NULL,
                                                                  NULL );
          if (delete_reply) g_variant_unref(delete_reply);
        }
     }

    g_variant_iter_free(sms_iter);
    g_variant_unref(reply);
    g_object_unref(system_bus);
    g_free(modem_path);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct SMS_VARS) );
    struct SMS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    MQTT_Subscribe ( module->MQTT_session, "SEND_SMS" );

    vars->sending_is_disabled = FALSE;                                               /* A l'init, l'envoi de SMS est autorisé */
    vars->ai_nbr_sms        = Mnemo_create_thread_AI ( module, "NBR_SMS", "Nombre de SMS envoyés", "sms", ARCHIVE_1_HEURE );
    vars->ai_signal_quality = Mnemo_create_thread_AI ( module, "SIGNAL_QUALITY", "Qualité du signal", "%", ARCHIVE_1_HEURE );
    vars->nbr_sms  = 0;
    gint next_read = 0;
    Envoyer_smsg_gsm_text ( module, "SMS System is running" );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */

/****************************************************** Ecoute du master ******************************************************/
       while ( module->Thread_run && module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *message = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, message );
          pthread_mutex_unlock ( &module->synchro );

          if (Json_has_member ( message, "token_lvl0" ))
           { gchar *token_lvl0 = Json_get_string ( message, "token_lvl0" );
             if (!strcasecmp (token_lvl0, "SEND_SMS") && Json_has_member ( message, "notif_sms" ) &&
                 Json_has_member ( message, "tech_id" ) && Json_has_member ( message, "acronyme" ) &&
                 Json_has_member ( message, "libelle" )
                )
              { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending msg '%s:%s' (%s)", thread_tech_id,
                          Json_get_string ( message, "tech_id" ), Json_get_string ( message, "acronyme" ),
                          Json_get_string ( message, "libelle" ) );
                Smsg_send_to_all_authorized_recipients( module, message );
              }
             else if (!strcasecmp (token_lvl0, "THREAD_TEST") && Json_has_member ( message, "test_mode" ) )
              { gchar *test_mode  = Json_get_string ( message, "test_mode" );
                if ( !strcasecmp ( test_mode, "test_gsm" ) ) Envoyer_smsg_gsm_text ( module, "Test SMS GSM OK !" );
                if ( !strcasecmp ( test_mode, "test_ovh" ) ) Envoyer_smsg_ovh_text ( module, "Test SMS OVH OK !" );
              }
           }
          Json_node_unref(message);
        }
/****************************************************** Lecture de SMS ********************************************************/
       if (Partage->top < next_read) continue;
       gdouble signal_quality;
       if (module->Thread_run && Smsg_get_signal_quality(module, &signal_quality))
        { Thread_send_comm_to_master ( module, TRUE );
          MQTT_Send_AI ( module, vars->ai_signal_quality, signal_quality, TRUE );
          Lire_sms_gsm(module);
        }
       else
        { Thread_send_comm_to_master ( module, FALSE ); }
       next_read = Partage->top + 50;
     }
    g_clear_pointer ( &vars->mm_modem_path, g_free );
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
