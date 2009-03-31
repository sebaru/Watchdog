/**********************************************************************************************************/
/* Commun/unite.c        Gestion des unites entreeANAs de Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 20 fév 2006 17:42:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Cst_entreeana.h"

 static gchar *UNITE_EA[NBR_TYPE_UNITE] =
  { "°C",
    "°K",
    "mbar",
    "bar",
    "m",
    "km",
    "l",
    "m/s",
    "km/h",
    "%",
    "% HR",
    "s",
    "h",
    "A"
  };

/**********************************************************************************************************/
/* Unite_vers_string: renvoie l'unite sous forme de chaine de caractere                                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gchar *Unite_vers_string ( guint type )
  { if (type<NBR_TYPE_UNITE) return( UNITE_EA[type] );
                        else return ( "Unknown" );
  }
/**********************************************************************************************************/
/* String_vers_unite: renvoie l'unite en tant que int                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gint String_vers_unite ( guchar *unite )
  { gint i;
    for (i=0; i<NBR_TYPE_UNITE; i++)
     { if ( !strcmp( UNITE_EA[i], unite ) ) return(i); }
    return(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
