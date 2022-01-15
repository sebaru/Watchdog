document.addEventListener('DOMContentLoaded', Load_page, false);

 function COMMAND_TEXT_Refresh ( )
  { $('#idTableTXT').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DI ( )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'COMMAND_TEXT',
         tech_id    : $('#idModalEditSelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditSelectAcronyme').val().toUpperCase(),
         map_tag    : $('#idModalEditTXTTag').val().toUpperCase(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableTXT').DataTable().ajax.reload(null, false);
     }, null);
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function TXTMap_Update_Choix_Acronyme ()
  { Common_Updater_Choix_Acronyme ( 'idModalEdit', 'DI' );
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function TXTMap_Update_Choix_Tech_ID ( def_tech_id, def_acronyme )
  { Common_Updater_Choix_TechID ( 'idModalEdit', 'DI', def_tech_id, def_acronyme );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function COMMAND_TEXT_Map_DI ( id )
  { table = $('#idTableTXT').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return item.id==id } )[0];
    $('#idMODALMapTitre').text( "Mapper le texte '"+selection.thread_acronyme+"'" );
    $('#idMODALMapRechercherTechID').off("input").on("input", function () { Common_Updater_Choix_TechID ( "idMODALMap", "DI" ); } );
    Common_Updater_Choix_TechID ( "idMODALMap", "DI", selection.tech_id, selection.acronyme );
    $('#idMODALMapValider').off("click").on( "click", function ()
     { $('#idMODALMap').modal("hide");
       COMMON_Map ( "_COMMAND_TEXT", selection.thread_acronyme,
                    $('#idMODALMapSelectTechID').val(),  $('#idMODALMapSelectAcronyme').val()
                  ).then ( () => { COMMAND_TEXT_Refresh(); } );
     });
    $('#idMODALMap').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {

    $('#idTableTXT').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map",	type : "GET", dataSrc: "mappings", data: { "thread_tech_id": "_COMMAND_TEXT" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "thread_acronyme", "title":"Texte Source", "className": "align-middle text-center" },
            { "data": null, "title":"Mapped on", "className": "align-middle text-center",
              "render": function (item)                { if(item.tech_id)
                   { return ( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) +":" + item.acronyme
                              + ", " + item.libelle );
                   } else return( "--" );
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Mapper cette commande", "COMMAND_TEXT_Map_DI", item.id, "directions", null );
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
