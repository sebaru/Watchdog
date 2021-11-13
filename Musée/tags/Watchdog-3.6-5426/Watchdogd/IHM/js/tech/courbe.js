 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    Charger_une_courbe ( "idChartCourbe", vars[3], vars[4], vars[5] )
	 }
