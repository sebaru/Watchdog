/**********************************************************************************************************/
/* Commun/Reseaux/Ssl.c          Gestion des connexions securisées                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 23 jun 2003 11:30:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include <stdio.h>
 #include <openssl/ssl.h>

/**********************************************************************************************************/
/* Nom_certif_signataire: renvoie le common name du signataire du certificat en parametre                 */
/* Entrée: un certificat x509                                                                             */
/* Sortie: une chaine non freeable                                                                        */
/**********************************************************************************************************/
 gchar *Nom_certif_signataire( X509 *certif )
  { X509_NAME_ENTRY *entry;
    X509_NAME *name;
    gint pos;

    name = X509_get_issuer_name ( certif );
    pos = X509_NAME_get_index_by_NID( name, NID_commonName, -1 );
    entry = X509_NAME_get_entry( name, pos );
    return ( ASN1_STRING_data( X509_NAME_ENTRY_get_data( entry ) ) );
  }
/**********************************************************************************************************/
/* Nom_certif: renvoie le common name du sujet du certificat en parametre                                 */
/* Entrée: un certificat x509                                                                             */
/* Sortie: une chaine non freeable                                                                        */
/**********************************************************************************************************/
 gchar *Nom_certif( X509 *certif )
  { X509_NAME_ENTRY *entry;
    X509_NAME *name;
    gint pos;

    name = X509_get_subject_name ( certif );
    pos = X509_NAME_get_index_by_NID( name, NID_commonName, -1 );
    entry = X509_NAME_get_entry( name, pos );
    return ( ASN1_STRING_data( X509_NAME_ENTRY_get_data( entry ) ) );
  }

/**********************************************************************************************************/
