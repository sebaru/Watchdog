/******************************************************************************************************************************/
/* Watchdogd/Http/getfile.c       Gestion des requests sur des ressources fichiers                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    10.09.2020 08:31:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getfile.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <fcntl.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
 extern struct HTTP_CONFIG Cfg_http;

/******************************************************************************************************************************/
/* Http_traiter_mnemos_validate: Valide la presence ou non d'un tech_id/acronyme dans le dico                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_upload ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                            SoupClientContext *client, gpointer user_data )
  { GBytes *request_brute;
    gchar filename[80], chaine[256];
    gsize taille;

    if (msg->method != SOUP_METHOD_POST)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );
    if (!Http_check_session( msg, session, 6 )) return;

    g_object_get ( msg, "request-body-data", &request_brute, NULL );
    gpointer data = g_bytes_get_data ( request_brute, &taille );

    gchar *filename_src = g_hash_table_lookup ( query, "filename" );
    if (!filename_src) return;
    if ( g_strrstr(filename_src, "..")) return;
    if (!g_str_has_suffix(filename_src, ".jpg")) return;

    gchar *filename_min = g_utf8_strdown ( filename_src, -1 );
    g_strcanon ( filename_min, "0123456789abcdefghijklmnopqrstuvwxyz._", '_' );
    g_snprintf ( filename, sizeof(filename), "Upload/%s", filename_min );
    g_free(filename_min);

    unlink(filename);
    gint fd = open ( filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR );
    if (fd==-1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s: File '%s' upload failed '%s''", __func__, filename, strerror(errno) );
       return;
     }

    write ( fd, data, taille );
    close(fd);

    gchar *thumb_src = g_hash_table_lookup ( query, "thumb" );
    if (thumb_src)
     { gint thumb=atoi(thumb_src);

       if(20<thumb && thumb<200)
        { g_snprintf( chaine, sizeof(chaine), "convert -thumbnail x%d %s %s", thumb, filename, filename );
          system(chaine);
        }
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
              "%s: File '%s' uploaded with size='%d'", __func__, filename, taille );

    Audit_log ( session, "File '%s' uploaded (size=%d)", filename, taille );
	   soup_message_set_status (msg, SOUP_STATUS_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_get_syn: Fourni une list JSON des elements d'un synoptique                                                    */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_traiter_file ( SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query,
                          SoupClientContext *client, gpointer user_data )
  { gint fd, taille_header, taille_fichier, taille_footer, taille_result;
    struct stat stat_buf;
    gchar fichier[80], header[80], footer[80], *result, *new_result;
    gboolean has_template;

    if (msg->method != SOUP_METHOD_GET)
     {	soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
		     return;
     }

    struct HTTP_CLIENT_SESSION *session = Http_print_request ( server, msg, path, client );

    if (g_strrstr(path, "..")) return;

    g_strcanon ( path, "ABCDEFGIHJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz._/", '_' );
    gchar *path_to_lower = g_utf8_strdown ( path, -1 );
    gchar **URI = g_strsplit ( path_to_lower, "/", -1 );
    g_free(path_to_lower);

    if (!URI)
     { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "URI Split Failed" );
       return;
     }

    taille_result = taille_header = taille_footer = 0;
    result = NULL;
    has_template = FALSE;

    if (!strcasecmp( URI[1], "img"))
     { g_snprintf ( fichier, sizeof(fichier), "%s/IHM/img/%s", WTD_PKGDATADIR, URI[2] ); }
    else if (!strcasecmp( URI[1], "js"))
     { if (URI[3]) g_snprintf ( fichier, sizeof(fichier), "%s/IHM/js/%s/%s", WTD_PKGDATADIR, URI[2], URI[3] );
              else g_snprintf ( fichier, sizeof(fichier), "%s/IHM/js/%s", WTD_PKGDATADIR, URI[2] );
     }
    else if (!strcasecmp( URI[1], "install"))
     { if(Config.installed == FALSE) g_snprintf ( fichier, sizeof(fichier), "%s/IHM/install.php", WTD_PKGDATADIR );
       else
        { g_strfreev(URI);
          soup_message_set_redirect ( msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/" );
          return;
        }
     }
    else if (!Config.installed)
     { g_strfreev(URI);
       soup_message_set_redirect ( msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/install" );
       return;
     }
    else if (!strcasecmp( URI[1], "upload"))
     { g_snprintf ( fichier, sizeof(fichier), "Upload/%s", URI[2] ); }
    else if (!strcasecmp( URI[1], "audio"))
     { g_snprintf ( fichier, sizeof(fichier), "%s/IHM/audio/%s", WTD_PKGDATADIR, URI[2] ); }
    else if (!strcasecmp(URI[1], "tech"))
     { if (!Http_check_session( msg, session, 6 ))
        { g_strfreev(URI);
          soup_message_set_redirect ( msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/login" );
          return;
        }
       g_snprintf ( header, sizeof(header), "%s/IHM/Tech/header.php", WTD_PKGDATADIR );
       g_snprintf ( footer, sizeof(footer), "%s/IHM/Tech/footer.php", WTD_PKGDATADIR );
       has_template = TRUE;
       g_snprintf ( fichier, sizeof(fichier), "%s/IHM/Tech/%s.php", WTD_PKGDATADIR, (URI[2] ? URI[2] : "dashboard") );

     }
    else if (!strcasecmp(URI[1], "home" ))
     { if ( !Http_check_session( msg, session, 0 ))
        { g_strfreev(URI);
          soup_message_set_redirect ( msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/login" );
          return;
        }
       g_snprintf ( header, sizeof(header), "%s/IHM/Home/header.php", WTD_PKGDATADIR );
       g_snprintf ( footer, sizeof(footer), "%s/IHM/Home/footer.php", WTD_PKGDATADIR );
       has_template = TRUE;
       g_snprintf ( fichier, sizeof(fichier), "%s/IHM/Home/%s.php", WTD_PKGDATADIR, URI[2] );
     }
    else if (!session && !strcasecmp( URI[1], "login"))
     { g_snprintf ( fichier, sizeof(fichier), "%s/IHM/login.php", WTD_PKGDATADIR ); }
    else if (!session)
     { g_strfreev(URI);
       soup_message_set_redirect ( msg, SOUP_STATUS_TEMPORARY_REDIRECT, "/login" );
       return;
     }
    else
     { g_snprintf ( header, sizeof(header), "%s/IHM/Home/header.php", WTD_PKGDATADIR );
       g_snprintf ( footer, sizeof(footer), "%s/IHM/Home/footer.php", WTD_PKGDATADIR );
       has_template = TRUE;
       g_snprintf ( fichier, sizeof(fichier), "%s/IHM/Home/index.php", WTD_PKGDATADIR );
     }
    g_strfreev(URI);
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG, "%s : Serving file %s", __func__, fichier );

/*--------------------------------------------------- LEcture header ---------------------------------------------------------*/
    if (has_template)
     { if (stat (header, &stat_buf)==-1)
        { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Header Error Stat" );
          return;
        }
       taille_header = stat_buf.st_size;
       taille_result += stat_buf.st_size;
       new_result = g_try_realloc ( result, taille_result );
       if (!new_result)
        { g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Header Memory Error" );
          return;
        } else result = new_result;
       fd = open ( header, O_RDONLY );
       if (fd==-1)
        { g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Header File Open Error" );
          return;
        }
       read ( fd, result, taille_header );
       close(fd);
     }

/*--------------------------------------------------- Lecture fichier --------------------------------------------------------*/
    if (stat (fichier, &stat_buf)==-1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : File '%s' not found", __func__, fichier );
       soup_message_set_status_full ( msg, SOUP_STATUS_NOT_FOUND, "File not found" );
       g_free(result);
       return;
     }

    taille_fichier = stat_buf.st_size;
    taille_result += stat_buf.st_size;
    new_result = g_try_realloc ( result, taille_result );
    if (!new_result)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : File '%s' Realloc error", __func__, fichier );
       g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Memory Error" );
       return;
     } else result = new_result;

   fd = open ( fichier, O_RDONLY );
    if (fd==-1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR, "%s : File '%s' open error '%s'", __func__, fichier, strerror(errno) );
       g_free(result);
       soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "File Open Error" );
       return;
     }
    read ( fd, result + taille_header, taille_fichier );
    close(fd);

/*--------------------------------------------------- LEcture footer ---------------------------------------------------------*/
    if (has_template)
     { if (stat (footer, &stat_buf)==-1)
        { soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Footer Error Stat" );
          return;
        }
       taille_footer = stat_buf.st_size;
       taille_result += stat_buf.st_size;
       new_result = g_try_realloc ( result, taille_result );
       if (!new_result)
        { g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Footer Memory Error" );
          return;
        } else result = new_result;
       fd = open ( footer, O_RDONLY );
       if (fd==-1)
        { g_free(result);
          soup_message_set_status_full ( msg, SOUP_STATUS_INTERNAL_SERVER_ERROR, "Footer File Open Error" );
          return;
        }
       read ( fd, result + taille_header + taille_fichier, taille_footer );
       close(fd);
     }
/*************************************************** Envoi au client **********************************************************/
	   soup_message_set_status (msg, SOUP_STATUS_OK);
         if ( g_str_has_suffix (path, ".js") )
     { soup_message_set_response ( msg, "text/javascript; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
    else if ( g_str_has_suffix (path, ".png") )
     { soup_message_set_response ( msg, "image/png; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
    else if ( g_str_has_suffix (path, ".svg") )
     { soup_message_set_response ( msg, "image/svg+xml; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
    else if ( g_str_has_suffix (path, ".jpg") )
     { soup_message_set_response ( msg, "image/jpeg; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
    else if ( g_str_has_suffix (path, ".webp") )
     { soup_message_set_response ( msg, "image/webp; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
    else if ( g_str_has_suffix (path, ".wav") )
     { soup_message_set_response ( msg, "audio/x-wav", SOUP_MEMORY_TAKE, result, taille_result ); }
    else
     { soup_message_set_response ( msg, "text/html; charset=UTF-8", SOUP_MEMORY_TAKE, result, taille_result ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
