
 var TableauDIV ="<div class='row mx-1 mb-1'>"+
                 "  <h3><i class='fas fa-chart-line text-primary'></i> <strong id='idTableauTitle'></strong></h3>"+
                 "  <div class ='ml-auto btn-group align-items-start'>"+
                 "   <i class='fas fa-clock text-primary mt-2 mr-2'></i>"+
                 "   <select id='idTableauPeriod' class='custom-select' onchange='Tableau_Set_Period()'>"+
                 "    <option value='HOUR'>Heure</option>"+
                 "    <option value='DAY'>Jour</option>"+
                 "    <option value='WEEK'>Semaine</option>"+
                 "    <option value='MONTH'>Mois</option>"+
                 "    <option value='YEAR'>Année</option>"+
                 "    <option value='ALL'>Tout</option>"+
                 "   </select>"+
                 "  </div>"+
                 "</div>"+
                 "<div class='row mx-1'><canvas id='idTableauCanvas' class='wtd-courbe'></canvas></div>";

 var Tableau_ID;

/********************************************* Appelé au chargement de la page ************************************************/
 function Charger_un_tableau ( tableau_id, period )
  { Send_to_API ( "GET", "/api/tableau/map/list", "tableau_id="+tableau_id, function(Response)
     { $('#idTableauTitle').text(Response.titre);
       Charger_plusieurs_courbes ( "idTableauCanvas", Response.tableau_map, period );
       $('#toplevel').slideDown("slow");
     }, null );
  }

/********************************************* Appelé au changement de periode ************************************************/
 function Tableau_Set_Period ()
  { Charger_un_tableau ( Tableau_ID, $("#idTableauPeriod").val() ); }

/********************************************* Charge la page entiere *********************************************************/
 function Charger_page_tableau ( tableau_id )
  { if (tableau_id==undefined) tableau_id=10000;
    Tableau_ID = tableau_id;

    Scroll_to_top();
    $('#toplevel').slideUp("normal", function ()
     { $('#toplevel').empty().append(TableauDIV);
       Charger_un_tableau ( Tableau_ID, "HOUR" );
     });
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
