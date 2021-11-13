 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Tableau_Set_Period ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/home/tableau/"+vars[3]+"/"+$("#idTableauPeriod").val() );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    Send_to_API ( "GET", "/api/tableau/list", null, function(Response)
     { tableau = Response.tableaux.filter( function(item) { return item.id==vars[3] } )[0];
       $('#idTableauTitle').text(tableau.titre);
     }, null );

    if (vars[4] != undefined)
     { $("#idTableauPeriod").val(vars[4]); }
    else { $("#idTableauPeriod").val("HOUR"); }

    Send_to_API ( "GET", "/api/tableau/map/list", "tableau_id="+vars[3], function(Response)
     { Charger_plusieurs_courbes ( "idChart", Response.tableau_map, vars[4] );
     }, null );

/* Charger les tableaux */

/*    $('#idTableTableau').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/tableau/list",	type : "GET", dataSrc: "tableaux",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": "id", "title":"#", "className": "text-center " },
            { "data": null, "title":"Level", "className": "align-middle  text-center",
              "render": function (item)
                { return( Select_Access_level ( "idTableauLevel_"+item.id,
                                                "Tableau_Set('"+item.id+"')",
                                                item.access_level )
                        );
                }
            },
            { "data": null, "title":"Titre", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "text", "idTableauTitre_"+item.id,
                                  "Tableau_Set('"+item.id+"')",
                                  "Quel est le titre du tableau ?",
                                  item.titre )
                        );
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Voir le tableau", "Redirect", "/home/tableau?id="+item.id, "chart-line", null );
                  boutons += Bouton_actions_add ( "primary", "Editer les courbes", "Redirect", "/tech/tableau_map/"+item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer ce tableau", "Tableau_Delete", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         /*responsive: true,*/
       /*}
     );*/
  }
