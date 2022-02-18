 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_New ( )
  { $('#idModalEditTitre').text ( "Ajouter un tableau" );
    Select_from_api ( "idModalEditPage", "/api/syn/list", null, "synoptiques", "syn_id", function (item)
     { return ( item.page+" - "+htmlEncode(item.libelle) ); }, Get_url_parameter("syn_id") );

    $('#idModalEditLibelle').val( "" );
    $('#idModalEditValider').attr( "onclick", "Tableau_Set(null)" );
    $('#idModalEdit').modal("show");
  }
/************************************ Créé un nouveau tableau *****************************************************************/
 function Tableau_Set ( tableau_id )
  { table = $('#idTableTableau').DataTable();
    selection = table.ajax.json().tableaux.filter( function(item) { return item.tableau_id==tableau_id } )[0];
    var json_request =
       { titre:  $('#idModalEditLibelle').val(),
         syn_id: parseInt($('#idModalEditPage').val()),
       }
    if (tableau_id!=null) json_request.tableau_id = tableau_id;

    Send_to_API ( "POST", "/api/tableau/set", JSON.stringify(json_request), function (Response)
     { $('#idTableTableau').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Tableau_Valide_delete ( tableau_id )
  { var json_request = JSON.stringify(
       { tableau_id: tableau_id
       }
     );
    Send_to_API ( "DELETE", "/api/tableau/del", json_request, function (Response)
     { $('#idTableTableau').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Tableau_Delete ( tableau_id )
  { table = $('#idTableTableau').DataTable();
    selection = table.ajax.json().tableaux.filter( function(item) { return item.tableau_id==tableau_id } )[0];
    Show_modal_del ( "Détruire le tableau ?",
                     "Etes-vous sur de vouloir supprimer le tableau suivant ?",
                     selection.titre,
                     function () { Tableau_Valide_delete(tableau_id); } );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Tableau_Edit ( tableau_id )
  { table = $('#idTableTableau').DataTable();
    selection = table.ajax.json().tableaux.filter( function(item) { return item.tableau_id==tableau_id } )[0];
    $('#idModalEditTitre').text ( "Modifier le tableau " + selection.titre + "?" );
    Select_from_api ( "idModalEditPage", "/api/syn/list", null, "synoptiques", "syn_id", function(item)
                        { return(item.page+" - "+htmlEncode(item.libelle)); }, selection.syn_id );

    $('#idModalEditLibelle').val( selection.titre );
    $('#idModalEditValider').attr( "onclick", "Tableau_Set('"+selection.tableau_id+"')" );
    $('#idModalEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { syn_id = Get_url_parameter("syn_id");
    if (syn_id != null)
     {  parametres = { "syn_id": syn_id }; }
    else parametres = {};
    $('#idTableTableau').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/tableau/list",	type : "GET", data: parametres, dataSrc: "tableaux",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "tableau_id",
         columns:
          [ { "data": "page", "title":"Page", "className": "align-middle text-center" },
            { "data": "titre", "title":"Titre", "className": "align-middle" },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  /*boutons += Bouton_actions_add ( "secondary", "Voir le tableau", "Redirect", "/"+item.page, "chart-line", null );*/
                  boutons += Bouton_actions_add ( "outline-primary", "Configurer", "Show_Modal_Tableau_Edit", item.tableau_id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-secondary", "Editer les courbes", "Redirect", "/tech/tableau_map?tableau_id="+item.tableau_id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer ce tableau", "Show_Modal_Tableau_Delete", item.tableau_id, "trash", null );
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
