document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( classe, selection )
  { $('#idModalDelTitre').text ( "Détruire le mapping ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer ce mapping ?"+
                                 "<hr>"+
                                 "<strong>"+selection.map_tech_id+":"+selection.map_tag +
                                 " <-> " + selection.tech_id + ":" + selection.acronyme + "</strong>" +
                                 "<br>" + selection.libelle
                                );
    $('#idModalDelValider').attr( "onclick",
                                  "Valider_Map_Del('idTableTXT','"+classe+"','"+selection.id+"')" );
    $('#idModalDel').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTableTXT').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DI ( )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'COMMAND_TXT',
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
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableTXT').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDI #idModalEditTitre').text ( "Editer MAP TXT" );
       TXTMap_Update_Choix_Tech_ID( selection.tech_id, selection.acronyme );
       $('#idModalEditDI #idModalEditTXTTag').val  ( selection.map_tag );
       $('#idModalEditDI #idModalEditTXTTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/process/smsg/list", null, function (Response)
        { $('#idModalEditTXTTechID').empty();
          $.each ( Response.TXTs, function ( i, TXT )
           { $('#idModalEditTXTTechID').append("<option value='"+TXT.tech_id+"'"+
                                                (TXT.tech_id == selection.tech_id ? "selected" : "")+">"+
                                                 TXT.tech_id+ " (" +TXT.description+")</option>"); } );
        });
     }
    else
     { $('#idModalEditDI #idModalEditTitre').text ( "Ajouter un mapping DI" );
       $('#idModalEditRechercherTechID').val ( '' );
       $('#idModalEditDI #idModalEditTXTTag').val     ( '' );
       $('#idModalEditDI #idModalEditTXTTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/process/smsg/list", null, function (Response)
        { $('#idModalEditTXTTechID').empty();
          $.each ( Response.TXTs, function ( i, TXT )
           { $('#idModalEditTXTTechID').append("<option value='"+TXT.tech_id+"'>"+TXT.tech_id+ " (" +TXT.description+")</option>"); } );
        });
       TXTMap_Update_Choix_Tech_ID( null, null );
     }
    $('#idModalEditDI').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {

    $('#idTableTXT').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "COMMAND_TEXT", "classe": "DI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ /*{ "data": "map_tech_id", "title":"TXT TechID", "className": "align-middle text-center" },*/
            { "data": "map_tag", "title":"Texte Source", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "->" ); }
            },
            { "data": null, "title":"DLS Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"DLS Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"DLS Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "Show_Modal_Map_Edit_DI", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet objet", "Show_Modal_Map_Del_DI", item.id, "trash", null );
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
