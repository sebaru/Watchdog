 document.addEventListener('DOMContentLoaded', Load_archive, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_archive ()
  { console.log ("in load archive !");
    vars = window.location.pathname.split('/');
    console.debug(vars);
    Charger_une_courbe ( "idChartArchive", vars[3], vars[4], vars[5] )
	 }
