/******************************************************************************************************************************/
/* Watchdogd/Http/getsyn.c       Gestion des request getsyn pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getsyn.c
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

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Traiter_dynxml: Traite une requete sur l'URI getsyn du webservice                                                          */
/* Entrées: la connexion websocket et la session                                                                              */
/* Sortie : FALSE si erreur                                                                                                   */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getsyn ( struct lws *wsi, struct HTTP_SESSION *session )
  { unsigned char header[256], *header_cur, *header_end;
    const char *content_type = "application/json";
 	  struct CMD_TYPE_SYNOPTIQUE *syndb;
    JsonBuilder *builder;
    JsonGenerator *gen;
    struct DB *db;
    gint retour;
    const gchar *id_syn_s;
   	gchar token_id[12];
    gchar requete[256];
    gint id_syn;
    gchar *buf;
    gsize taille_buf;

#ifdef bouh
    if ( session==NULL || session->util==NULL )
     { Http_Send_response_code ( wsi, 401 );
       return(TRUE);
     }
#endif

    id_syn_s   = lws_get_urlarg_by_name	( wsi, "id_syn=",   token_id,   sizeof(token_id) );
    if (id_syn_s) { id_syn = atoi ( id_syn_s ); }
    else { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST ); /* Bad Request */
           return(TRUE);
         }

    syndb = Rechercher_synoptiqueDB ( id_syn );
    if ( ! syndb )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                 "%s: (sid %s) Synoptique %d not found in DB", __func__, Http_get_session_id ( session ), id_syn );
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(FALSE);
     }

#ifdef bouh
    if ( Tester_level_util(session->util, syndb->access_groupe)==FALSE)
     { g_free(syndb);
       Http_Send_response_code ( wsi, 401 );
       return(TRUE);
     }
#endif
     
/************************************************ Préparation du buffer JSON **************************************************/
    builder = json_builder_new ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : (sid %s) JSon builder creation failed", __func__, Http_get_session_id ( session ) );
       g_free(syndb);
       Http_Send_response_code ( wsi, HTTP_SERVER_ERROR );
       return(TRUE);
     }
                                                          /* Lancement de la requete de recuperation du contenu du synoptique */
/*------------------------------------------------------- Dumping synoptique -------------------------------------------------*/
    /*json_builder_set_member_name  ( builder, "Synoptique" );  Pas de noeud principal */
    json_builder_begin_object (builder);                                                                  /* Contenu du Noeud */
    json_builder_set_member_name  ( builder, "id" );            json_builder_add_int_value    ( builder, syndb->id );
    json_builder_set_member_name  ( builder, "groupe" );        json_builder_add_string_value ( builder, syndb->groupe );
    json_builder_set_member_name  ( builder, "page" );          json_builder_add_string_value ( builder, syndb->page );
    json_builder_set_member_name  ( builder, "libelle" );       json_builder_add_string_value ( builder, syndb->libelle );
    g_free(syndb);

/*-------------------------------------------------------- Dumping passerelles -----------------------------------------------*/
    if ( Recuperer_passerelleDB( &db, id_syn ) )
     { struct CMD_TYPE_PASSERELLE *pass;
       json_builder_set_member_name  ( builder, "passerelles" );
       json_builder_begin_array (builder);                                                   /* Création du noeud Passerelles */
       while ( (pass = Recuperer_passerelleDB_suite( &db )) != NULL )
        { json_builder_begin_object (builder);                                                 /* Contenu du Noeud Passerelle */
          json_builder_set_member_name  ( builder, "id" );            json_builder_add_int_value    ( builder, pass->id );
          json_builder_set_member_name  ( builder, "syn_cible_id" );  json_builder_add_int_value    ( builder, pass->syn_cible_id );
          json_builder_set_member_name  ( builder, "libelle" );       json_builder_add_string_value ( builder, pass->libelle );
          json_builder_end_object (builder);                                                                /* End Passerelle */
          g_free(pass);
        }
       json_builder_end_array (builder);                                                              /* End Passerelle Array */
     }

/*-------------------------------------------------------- Dumping motifs  ---------------------------------------------------*/
    if ( Recuperer_motifDB( &db, id_syn ) )
     { struct CMD_TYPE_MOTIF *motif;
       json_builder_set_member_name  ( builder, "motifs" );
       json_builder_begin_array (builder);                                                         /* Création du noeud Motif */
       while( (motif = Recuperer_motifDB_suite( &db )) )
        { json_builder_begin_object (builder);                                                 /* Contenu du contenu du noeud */
          json_builder_set_member_name  ( builder, "id" );           json_builder_add_int_value    ( builder, motif->id );
          json_builder_set_member_name  ( builder, "libelle" );      json_builder_add_string_value ( builder, motif->libelle );
          json_builder_set_member_name  ( builder, "icone_id" );     json_builder_add_int_value    ( builder, motif->icone_id );
          json_builder_set_member_name  ( builder, "posx" );         json_builder_add_int_value    ( builder, motif->position_x );
          json_builder_set_member_name  ( builder, "posy" );         json_builder_add_int_value    ( builder, motif->position_y );
          json_builder_set_member_name  ( builder, "angle" );        json_builder_add_double_value ( builder, motif->angle );
          json_builder_set_member_name  ( builder, "type_gestion" ); json_builder_add_int_value    ( builder, motif->type_gestion );
          json_builder_set_member_name  ( builder, "bit_ctrl" );     json_builder_add_int_value    ( builder, motif->bit_controle );
          json_builder_set_member_name  ( builder, "etat" );         json_builder_add_int_value    ( builder, Partage->i[motif->bit_controle].etat );
          json_builder_set_member_name  ( builder, "rouge" );        json_builder_add_int_value    ( builder, Partage->i[motif->bit_controle].rouge );
          json_builder_set_member_name  ( builder, "vert" );         json_builder_add_int_value    ( builder, Partage->i[motif->bit_controle].vert );
          json_builder_set_member_name  ( builder, "bleu" );         json_builder_add_int_value    ( builder, Partage->i[motif->bit_controle].bleu );
          json_builder_set_member_name  ( builder, "cligno" );       json_builder_add_int_value    ( builder, Partage->i[motif->bit_controle].cligno );
          json_builder_end_object (builder);                                                                /* End Passerelle */
          g_free(motif);
        }
       json_builder_end_array (builder);                                                                  /* End Motifs Array */
     }

/*-------------------------------------------------------- Dumping scenario --------------------------------------------------*/
    if ( Recuperer_scenarioDB( &db, id_syn ) )
     { struct CMD_TYPE_SCENARIO *scenario;
       json_builder_set_member_name  ( builder, "scenarios" );
       json_builder_begin_array (builder);                                                         /* Création du noeud Motif */
       while( (scenario = Recuperer_scenarioDB_suite( &db )) )
        { json_builder_begin_object (builder);                                                 /* Contenu du contenu du noeud */
          json_builder_set_member_name  ( builder, "id" );           json_builder_add_int_value    ( builder, scenario->id );
          json_builder_set_member_name  ( builder, "syn_id" );       json_builder_add_int_value    ( builder, scenario->syn_id );
          json_builder_set_member_name  ( builder, "posx" );         json_builder_add_int_value    ( builder, scenario->posx );
          json_builder_set_member_name  ( builder, "posy" );         json_builder_add_int_value    ( builder, scenario->posy );
          json_builder_end_object (builder);                                                                /* End Passerelle */
          g_free(scenario);
        }
       json_builder_end_array (builder);                                                                  /* End Motifs Array */
     }

    json_builder_end_object (builder);                                                                        /* End Document */

    gen = json_generator_new ();                                                                      /* Creating JSON buffer */
    json_generator_set_root ( gen, json_builder_get_root(builder) );
    json_generator_set_pretty ( gen, TRUE );
    buf = json_generator_to_data (gen, &taille_buf);
    g_object_unref(builder);
    g_object_unref(gen);



#ifdef bouh

/*------------------------------------------- Dumping cadran --------------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping cadrans !!");
    if ( Recuperer_cadranDB( &db, id_syn ) )
     { for ( ; ; )
        { struct CMD_TYPE_CADRAN *cadran;
          gfloat valeur = 0.0;
          gchar *unite= NULL;
          cadran = Recuperer_cadranDB_suite( &db );
          if (!cadran) break;                                                              /* Terminé ?? */

          xmlTextWriterStartElement(writer, (const unsigned char *)"cadran");           /* Start Capteur */
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"id",      "%d", cadran->id );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"libelle", "%s", cadran->libelle );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"type", "%d",    cadran->type );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"num", "%d",     cadran->bit_controle );
          switch(cadran->type)
           { case MNEMO_ENTREE_ANA:
                  valeur = Partage->ea[cadran->bit_controle].val_ech;
                  unite =  Partage->ea[cadran->bit_controle].confDB.unite;
                  break;
             default: valeur = 0.0; unite = "?";
           }
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"valeur", "%f", valeur );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"unite", "%s", unite );
          xmlTextWriterEndElement(writer);                                              /* End passerelle */
          g_free(cadran);
        }
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping cadrans !!");
#endif


/*************************************************** Envoi au client **********************************************************/
    header_cur = header;
    header_end = header + sizeof(header);
    
    retour = lws_add_http_header_status( wsi, 200, &header_cur, header_end );
    retour = lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (const unsigned char *)content_type, strlen(content_type),
                                           &header_cur, header_end );
    retour = lws_add_http_header_content_length ( wsi, taille_buf, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    lws_write ( wsi, buf, taille_buf, LWS_WRITE_HTTP);                                                      /* Send to client */
    g_free(buf);                                                      /* Libération du buffer dont nous n'avons plus besoin ! */
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
