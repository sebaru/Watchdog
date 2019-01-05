/**********************************************************************************************************/
/* Commun/Reseaux/Ssl.c          Gestion des connexions securisées                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                      lun 23 jun 2003 11:30:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Ssl.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <stdio.h>
 #include <openssl/ssl.h>

 #define NOT_FOUND "Unknown"

/**********************************************************************************************************/
/* Nom_certif_signataire: renvoie le common name du signataire du certificat en parametre                 */
/* Entrée: un certificat x509                                                                             */
/* Sortie: une chaine non freeable                                                                        */
/**********************************************************************************************************/
 const gchar *Nom_certif_signataire( X509 *certif )
  { X509_NAME_ENTRY *entry;
    X509_NAME *name;
    gint pos;

    if (!certif) return(NOT_FOUND);
    name = X509_get_issuer_name ( certif );
    if (!name) return(NOT_FOUND);
    pos = X509_NAME_get_index_by_NID( name, NID_commonName, -1 );
    entry = X509_NAME_get_entry( name, pos );
    if (!entry) return(NOT_FOUND);
    return ( ASN1_STRING_get0_data( X509_NAME_ENTRY_get_data( entry ) ) );
  }
/**********************************************************************************************************/
/* Nom_certif: renvoie le common name du sujet du certificat en parametre                                 */
/* Entrée: un certificat x509                                                                             */
/* Sortie: une chaine non freeable                                                                        */
/**********************************************************************************************************/
 const gchar *Nom_certif( X509 *certif )
  { X509_NAME_ENTRY *entry;
    X509_NAME *name;
    gint pos;

    if (!certif) return(NOT_FOUND);
    name = X509_get_subject_name ( certif );
    if (!name) return(NOT_FOUND);
    pos = X509_NAME_get_index_by_NID( name, NID_commonName, -1 );
    entry = X509_NAME_get_entry( name, pos );
    if (!entry) return(NOT_FOUND);
    return ( ASN1_STRING_get0_data( X509_NAME_ENTRY_get_data( entry ) ) );
  }

/**********************************************************************************************************/
