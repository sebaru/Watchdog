document.addEventListener('DOMContentLoaded', Load_page, false);

 function COMMAND_TEXT_Refresh ( )
  { $('#idTableTXT').DataTable().ajax.reload(null, false);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function COMMAND_TEXT_Add ( )
  { var json_request = JSON.stringify( { thread_tech_id: "_COMMAND_TEXT", thread_acronyme: $('#idModalCommandTextAdd').val().toUpperCase() } );
    Send_to_API ( 'POST', "/api/map", json_request, function () { COMMAND_TEXT_Refresh (); });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function COMMAND_TEXT_Map_DI ( mappings_id )
  { table = $('#idTableTXT').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return item.mappings_id==mappings_id } )[0];
    $('#idMODALMapTitre').text( "Mapper le texte '"+selection.thread_acronyme+"'" );
    $('#idMODALMapRechercherTechID').off("input").on("input", function () { Common_Updater_Choix_TechID ( "idMODALMap", "DI" ); } );
    Common_Updater_Choix_TechID ( "idMODALMap", "DI", selection.tech_id, selection.acronyme );
    $('#idMODALMapValider').off("click").on( "click", function ()
     { $('#idMODALMap').modal("hide");
       COMMON_Map ( "_COMMAND_TEXT", selection.thread_acronyme,
                    $('#idMODALMapSelectTechID').val(),  $('#idMODALMapSelectAcronyme').val()
                  );
       COMMAND_TEXT_Refresh();
     });
    $('#idMODALMap').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {

    $('#idTableTXT').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "mappings_id", paging: false,
         ajax: {	url : "/api/map",	type : "GET", dataSrc: "mappings", data: { "thread_tech_id": "_COMMAND_TEXT" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "thread_acronyme", "title":"Texte Source", "className": "align-middle text-center" },
            { "data": null, "title":"Mapped on", "className": "align-middle text-center",
              "render": function (item)                { if(item.tech_id)
                   { return ( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) +":" + item.acronyme );
                   } else return( "--" );
                }
            },
            { "data": null, "title":"Description", "className": "align-middle text-center",
              "render": function (item)
                { if(item.tech_id) { return ( item.libelle ); } else return( "--" ); }
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Mapper cette commande", "COMMAND_TEXT_Map_DI", item.mappings_id, "directions", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            },
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
