 document.addEventListener('DOMContentLoaded', Load_dashboard, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_dashboard ()
  { console.log ("in load dashboard !");
    Charger_une_courbe ( "idCourbeDlsTourParSec", "SYS", "DLS_TOUR_PER_SEC", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsBitParSec", "SYS", "DLS_BIT_PER_SEC", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsAttente", "SYS", "DLS_WAIT", "HOUR" );
    Charger_une_courbe ( "idCourbeNbArchive", "SYS", "ARCH_REQUEST_NUMBER", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsNbMSG", "SYS", "NBR_MSG_QUEUE", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsNbVISUEL", "SYS", "NBR_VISUEL_QUEUE", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsNbLigne", "SYS", "NBR_LIGNE_DLS", "MONTH" );
  }
