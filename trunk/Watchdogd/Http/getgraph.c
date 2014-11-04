/**********************************************************************************************************/
/* Watchdogd/Http/getgraph.c       Gestion des request getgif pour le thread HTTP de watchdog             */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 26 avril 2014 11:10:04 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * getgraph.c
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
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <microhttpd.h>
 #include <rrd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"
 #define MAX_RRD_COURBE  5
 static gchar *couleur[] = { "#0000FF", "#00FF00", "#FF0000", "#FFAA00", "#00FFFF" };

/**********************************************************************************************************/
/* Http_getgraph_create_graph : Lance la creation du fichier PNG associé aux RRA en parametre             */
/* Entrée : Le type de bit interne, son numéro, et la peride d'éffichage souhaitée                        */
/* Sortie : True si OK, False si Pb                                                                       */
/**********************************************************************************************************/
 static gboolean Http_getgraph_create_graph_ea ( gint num, const gchar *period, gint width )
  { gchar libelle[MAX_RRD_COURBE][80], unite[MAX_RRD_COURBE][20], rrd_file[MAX_RRD_COURBE][80];
    gchar def[MAX_RRD_COURBE][4][80], gprint[MAX_RRD_COURBE][5][80];
    gchar titre[80], vlabel[80], raxis[80], width_char[20];
    gchar *options[100], target_file[80];
    gchar requete[128], tableau[128];
    gint cpt_courbe, cpt, cpt_options;
    char **calcpr  = NULL;
    int xsize, ysize;
    double ymin, ymax;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                 "Http_getgraph_create_graph_ea: Database Connection Failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT tableau FROM eana,mnemos"
               " WHERE eana.id_mnemo=mnemos.id AND mnemos.type = %d AND mnemos.num = %d"
               " LIMIT 1",
               MNEMO_ENTREE_ANA, num );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Libere_DB_SQL( &db );
       return(FALSE);
     }

    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    g_snprintf( tableau, sizeof(tableau), "%s", db->row[0] );

    if (strlen(tableau))
     { gchar *tab_norm;
       tab_norm = Normaliser_chaine( tableau );                        /* Normalisation du nom du tableau */
       if (!tab_norm)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                    "Http_getgraph_create_graph_ea: Normalisation Tableau Failed (%s)", tableau );
          Libere_DB_SQL( &db );
          return(FALSE);
        }
       g_snprintf( requete, sizeof(requete),
                  "SELECT libelle,unite,num FROM eana,mnemos "
                  "WHERE eana.id_mnemo=mnemos.id AND tableau = '%s'", tab_norm );
       g_free(tab_norm);
     }
    else
     { g_snprintf( requete, sizeof(requete),
                  "SELECT libelle,unite,num FROM eana,mnemos "
                  "WHERE eana.id_mnemo=mnemos.id AND mnemos.type = %d AND mnemos.num = %d",
                  MNEMO_ENTREE_ANA, num );
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )                     /* Execution de la requete SQL */
     { Libere_DB_SQL( &db );
       return(FALSE);
     }

   cpt_courbe = 0;
   while(Recuperer_ligne_SQL(db) && cpt_courbe<MAX_RRD_COURBE)         /* Chargement d'une ligne resultat */
    { g_snprintf( libelle [cpt_courbe], sizeof(libelle [cpt_courbe]), "%s", db->row[0]);
      g_snprintf( unite   [cpt_courbe], sizeof(unite   [cpt_courbe]), "%s", db->row[1]);
      g_snprintf( rrd_file[cpt_courbe], sizeof(rrd_file[cpt_courbe]),
                  "RRA/%02d-%04d.rrd", MNEMO_ENTREE_ANA, atoi(db->row[2]) );
      cpt_courbe++;
    }
   Libere_DB_SQL( &db );                                          /* On a terminé avec la base de données */

   cpt_options = 0;
   options[cpt_options++] = "rrdgraph";
   g_snprintf( target_file, sizeof(target_file), "WEB/img/%02d-%04d-%s.png", MNEMO_ENTREE_ANA, num, period );
   options[cpt_options++] = target_file;
   options[cpt_options++] = "-aPNG";
   options[cpt_options++] = "--slope-mode";
        if ( ! strcmp (period, "hour"  ) ) { options[cpt_options++] = "--start=-2hour";  }
   else if ( ! strcmp (period, "day"   ) ) { options[cpt_options++] = "--start=-1day";   }
   else if ( ! strcmp (period, "week"  ) ) { options[cpt_options++] = "--start=-7days";  }
   else if ( ! strcmp (period, "month" ) ) { options[cpt_options++] = "--start=-1month"; }
   else if ( ! strcmp (period, "year"  ) ) { options[cpt_options++] = "--start=-1year";  }

   if (strlen(tableau)) g_snprintf( titre, sizeof(titre), "--title=%s", tableau );
                   else g_snprintf( titre, sizeof(titre), "--title=%s", libelle[0] );
   options[cpt_options++] = titre;

   g_snprintf( vlabel, sizeof(vlabel), "--vertical-label=%s", unite[0] );
   options[cpt_options++] = vlabel;
   if (width)
    { g_snprintf( width_char, sizeof(width_char), "--width=%d", width );
      options[cpt_options++] = width_char;
    }
   else options[cpt_options++] = "--width=1200";
   options[cpt_options++] = "--height=300";
   options[cpt_options++] = "--full-size-mode";
   g_snprintf( raxis, sizeof(raxis), "--right-axis-label=%s", unite[0] );
   options[cpt_options++] = raxis;
   options[cpt_options++] = "--right-axis=1:0";

   if (strlen(tableau))
    { cpt = 0;
      while (cpt < cpt_courbe)
       { g_snprintf( def[cpt][0], sizeof(def[cpt][0]),
                     "DEF:v%dval=%s:val:AVERAGE", cpt, rrd_file[cpt] );
         options[cpt_options++] = def[cpt][0];
         g_snprintf( def[cpt][1], sizeof(def[cpt][1]),
                     "DEF:m%dval=%s:val:MIN", cpt, rrd_file[cpt] );
         options[cpt_options++] = def[cpt][1];
         g_snprintf( def[cpt][2], sizeof(def[cpt][2]),
                     "DEF:M%dval=%s:val:MAX", cpt, rrd_file[cpt] );
         options[cpt_options++] = def[cpt][2];
         g_snprintf( def[cpt][3], sizeof(def[cpt][3]),
                     "DEF:L%dval=%s:val:LAST", cpt, rrd_file[cpt] );
         options[cpt_options++] = def[cpt][3];
         cpt++;
       }

      cpt = 0;
      while (cpt < cpt_courbe)
       {
    //"AREA:Mval#770000:Max",
         g_snprintf( gprint[cpt][0], sizeof(gprint[cpt][0]),
                     "LINE%d:v%dval%s:%s en %s",
                     cpt+1, cpt, couleur[cpt], libelle[cpt], unite[cpt] );
         options[cpt_options++] = gprint[cpt][0];
         g_snprintf( gprint[cpt][1], sizeof(gprint[cpt][1]),
                    "GPRINT:m%dval:MIN:Min Value %%6.2lf", cpt );
         options[cpt_options++] = gprint[cpt][1];
         g_snprintf( gprint[cpt][2], sizeof(gprint[cpt][2]),
                    "GPRINT:M%dval:MAX:Max Value %%6.2lf", cpt );
         options[cpt_options++] = gprint[cpt][2];
         g_snprintf( gprint[cpt][3], sizeof(gprint[cpt][3]),
                    "GPRINT:v%dval:AVERAGE:Moyenne Value %%6.2lf", cpt );
         options[cpt_options++] = gprint[cpt][3];
         g_snprintf( gprint[cpt][4], sizeof(gprint[cpt][4]),
                    "GPRINT:L%dval:LAST:Last Value %%6.2lf", cpt );
         options[cpt_options++] = gprint[cpt][4];
         cpt++;
       }
    }
   else
    { g_snprintf( def[0][0], sizeof(def[0][0]),
                    "DEF:vval=%s:val:AVERAGE", rrd_file[0] );
      options[cpt_options++] = def[0][0];
      g_snprintf( def[0][1], sizeof(def[0][1]),
                  "DEF:mval=%s:val:MIN", rrd_file[0] );
      options[cpt_options++] = def[0][1];
      g_snprintf( def[0][2], sizeof(def[0][2]),
                  "DEF:Mval=%s:val:MAX", rrd_file[0] );
      options[cpt_options++] = def[0][2];
      g_snprintf( def[0][3], sizeof(def[0][3]),
                  "DEF:Lval=%s:val:LAST", rrd_file[0] );
      options[cpt_options++] = def[0][3];
   //"AREA:Mval#770000:Max",
      g_snprintf( gprint[0][0], sizeof(gprint[0][0]),
                    "LINE1:vval%s:%s en %s",
                    couleur[0], libelle[0], unite[0] );
      options[cpt_options++] = gprint[0][0];
      options[cpt_options++] = "GPRINT:mval:MIN:Min Value %6.2lf";
      options[cpt_options++] = "GPRINT:Mval:MAX:Max Value %6.2lf";
      options[cpt_options++] = "GPRINT:vval:AVERAGE:Moyenne Value %6.2lf";
      options[cpt_options++] = "GPRINT:Lval:LAST:Last Value %6.2lf";
    }
   options[cpt_options] = NULL;
   Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
            "Http_getgraph_create_graph_ea: Creation of file %s in progress", target_file );
   for (cpt=0; options[cpt]; cpt++)
    { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
               "Http_getgraph_create_graph_ea: RrdGraph Options : %s", options[cpt] );
    }      
   rrd_clear_error();
   gboolean result;
   optind = opterr = 0;
   rrd_clear_error();
   result = rrd_graph(cpt_options, options, &calcpr, &xsize, &ysize, NULL, &ymin, &ymax);
   if (result)       
    { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_getgraph_create_graph_ea: Error %d creating RRD (%s).", result, rrd_get_error() );
      return(FALSE);
    }
   Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_getgraph_create_graph_ea: Creation of PNG file %s OK", target_file );
   return(TRUE);
 }
/**********************************************************************************************************/
/* Http_Traiter_request_getgraph: Traite une requete sur l'URI getgraph                                   */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gint Http_Traiter_request_getgraph ( struct HTTP_SESSION *session, struct MHD_Connection *connection )
  { const gchar *type_char, *num_char, *period_char, *width_char;
    struct MHD_Response *response;
    gint type, num, fd, width;
    gchar nom_fichier[80];
    struct stat sbuf;

    type_char   = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "type"   );
    num_char    = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "num"    );
    period_char = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "period" );
    width_char  = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "width" );

    if (!type_char || !num_char || !period_char)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getgraph : Error /getgraph error Wrong parameters" );
       return(FALSE);
     }

    type = atoi(type_char);
    num  = atoi(num_char);
    if(!width_char) width = 0;
               else width = atoi(width_char);

    if (type == MNEMO_ENTREE_ANA)                                         /* Si le type est un entree_ana */
     { if (Http_getgraph_create_graph_ea ( num, period_char, width ) == FALSE)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                   "Http_Traiter_request_getgraph : Error with graph Creation for EA" );
          return(FALSE);
        }
       else
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Http_Traiter_request_getgraph : Graph created" );
        }
     }

    g_snprintf( nom_fichier, sizeof(nom_fichier), "WEB/img/%02d-%04d-%s.png", type, num, period_char );
    fd = open ( nom_fichier, O_RDONLY);
    if ( fd == -1 || fstat (fd, &sbuf) == -1)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_request : Error /getgraph error %s on file %s", strerror(errno), nom_fichier );
       if (fd!=-1) close(fd);
       return(FALSE);
     }

    response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", "image/png");
    Http_Add_response_header ( response );
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
