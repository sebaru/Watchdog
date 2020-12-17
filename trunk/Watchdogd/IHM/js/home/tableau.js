 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.debug(vars);
    var json_request = JSON.stringify( { id : vars[3] } );
    Send_to_API ( "PUT", "/api/tableau/map/get", json_request, function(Response)
     { console.debug(Response);
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
          [ { "data": "id", "title":"#", "className": "text-center hidden-xs" },
            { "data": null, "title":"Level", "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { return( Select_Access_level ( "idTableauLevel_"+item.id,
                                                "Tableau_Set('"+item.id+"')",
                                                item.access_level )
                        );
                }
            },
            { "data": null, "title":"Titre", "className": "align-middle hidden-xs",
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
