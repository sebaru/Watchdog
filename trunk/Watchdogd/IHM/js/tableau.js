
 var TableauDIV ="<div class='row'>"+
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
                 "<div class='row'><canvas id='idTableauCanvas' class='wtd-courbe'></canvas></div>";

/********************************************* Appelé au chargement de la page ************************************************/
 function Tableau_Set_Period ()
  { console.log("Tableau_Set_Period");
    /*vars = window.location.pathname.split('/');
    Redirect ( "/tableau/"+vars[2]+"/"+$("#idTableauPeriod").val() );*/
  }
 function Charger_page_tableau ( tableau_id )
  { if (tableau_id==undefined) tableau_id=10000;
    $('#toplevel').fadeOut("fast", function()
     { $('#toplevel').empty().append(TableauDIV);
       Send_to_API ( "GET", "/api/tableau/map/list", "tableau_id="+tableau_id, function(Response)
        { $('#idTableauTitle').text(Response.titre);
          Charger_plusieurs_courbes ( "idTableauCanvas", Response.tableau_map, "HOUR" );
          $('#toplevel').fadeIn("slow");
        }, null );
       console.log("loaded");
     });
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
