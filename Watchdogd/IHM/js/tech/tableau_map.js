 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_Map_New ( )
  { var json_request = JSON.stringify(
       { tableau_id: Get_url_parameter("tableau_id"),
         tech_id : "New_tech_id",
         acronyme: "New_acronyme",
         color   : "#00F",
       }
     );
    Send_to_API ( "POST", "/api/tableau/map/set", json_request, function (Response)
     { $('#idTableTableauMap').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_Map_Set ( map_id )
  { table = $('#idTableTableauMap').DataTable();
    selection = table.ajax.json().tableau_map.filter( function(item) { return item.id==map_id } )[0];
    var json_request = JSON.stringify(
       { id: map_id,
         tech_id : $('#idTableauMapTechId_'+map_id).val(),
         acronyme: $('#idTableauMapAcronyme_'+map_id).val(),
         color   : $('#idTableauMapColor_'+map_id).val(),
       }
     );
    Send_to_API ( "POST", "/api/tableau/map/set", json_request, function (Response)
     { $('#idTableTableauMap').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Tableau_Map_Valide_delete ( map_id )
  { var json_request = JSON.stringify(
       { id: map_id
       }
     );
    Send_to_API ( "DELETE", "/api/tableau/map/del", json_request, function (Response)
     { $('#idTableTableauMap').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Tableau_Map_Delete ( map_id )
  { table = $('#idTableTableauMap').DataTable();
    selection = table.ajax.json().tableau_map.filter( function(item) { return item.id==map_id } )[0];
    Show_modal_del ( "Supprimer la courbe du tableau ?",
                     "Etes-vous sur de vouloir supprimer la courbe suivante ?",
                     selection.tech_id+":"+selection.acronyme,
                     function () { Tableau_Map_Valide_delete(map_id); } );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/tableau/list", null, function(Response)
     { tableau = Response.tableaux.filter( function(item) { return item.id==Get_url_parameter("tableau_id") } )[0];
       $('#idTableauTitle').text(tableau.titre);
     }, null );

    $('#idTableTableauMap').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/tableau/map/list",	type : "GET", data: { tableau_id: Get_url_parameter("tableau_id") }, dataSrc: "tableau_map",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": null, "title":"TechID", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "text", "idTableauMapTechId_"+item.id,
                                  "Tableau_Map_Set('"+item.id+"')",
                                  "Quel est le tech_id du bit cible ?",
                                  item.tech_id )
                        );
                }
            },
            { "data": null, "title":"Acronyme", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "text", "idTableauMapAcronyme_"+item.id,
                                  "Tableau_Map_Set('"+item.id+"')",
                                  "Quel est l'acronyme du bit cible ?",
                                  item.acronyme )
                        );
                }
            },
            { "data": null, "title":"Couleur", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "text", "idTableauMapColor_"+item.id,
                                  "Tableau_Map_Set('"+item.id+"')",
                                  "Quel est l'acronyme du bit cible ?",
                                  item.color )
                        );
                }
            },
            { "data": "libelle", "title":"Libellé", "className": "align-middle text-center " },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Voir la courbe", "Redirect", "/tech/courbe/"+item.tech_id+"/"+item.acronyme+"/HOUR", "chart-line", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cette courbe", "Tableau_Map_Delete", item.id, "trash", null );
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
