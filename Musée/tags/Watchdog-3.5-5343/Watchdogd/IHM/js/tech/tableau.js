 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_New ( )
  { var json_request = JSON.stringify(
       { titre: "Nouveau tableau",
         access_level: 0
       }
     );
    Send_to_API ( "POST", "/api/tableau/set", json_request, function (Response)
     { $('#idTableTableau').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_Set ( id )
  { table = $('#idTableTableau').DataTable();
    selection = table.ajax.json().tableaux.filter( function(item) { return item.id==id } )[0];
    var json_request = JSON.stringify(
       { id: id,
         titre: $('#idTableauTitre_'+id).val(),
         access_level: $('#idTableauLevel_'+id).val(),
       }
     );
    Send_to_API ( "POST", "/api/tableau/set", json_request, function (Response)
     { $('#idTableTableau').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Tableau_Valide_delete ( id )
  { var json_request = JSON.stringify(
       { id: id
       }
     );
    Send_to_API ( "DELETE", "/api/tableau/del", json_request, function (Response)
     { $('#idTableTableau').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Tableau_Delete ( id )
  { table = $('#idTableTableau').DataTable();
    selection = table.ajax.json().tableaux.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Détruire le tableau ?",
                     "Etes-vous sur de vouloir supprimer le tableau suivant ?",
                     selection.titre,
                     "Tableau_Valide_delete("+id+")" );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableTableau').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/tableau/list",	type : "GET", dataSrc: "tableaux",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": null, "title":"Level", "className": "align-middle ",
              "render": function (item)
                { return( Select_Access_level ( "idTableauLevel_"+item.id, "Tableau_Set('"+item.id+"')", item.access_level )
                        );
                }
            },
            { "data": null, "title":"Titre", "className": "align-middle",
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
                  boutons += Bouton_actions_add ( "primary", "Voir le tableau", "Redirect", "/home/tableau/"+item.id, "chart-line", null );
                  boutons += Bouton_actions_add ( "primary", "Editer les courbes", "Redirect", "/tech/tableau_map/"+item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer ce tableau", "Tableau_Delete", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );
  }
