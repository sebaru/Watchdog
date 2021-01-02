 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_GSM_Del ( map_tech_id, map_tag )
  { var json_request = JSON.stringify(
       { classe     : type,
         map_tech_id: map_tech_id,
         map_tag    : map_tag
       }
     );
    Send_to_API ( "DELETE", "/api/map/del", json_request, function(Response)
     { $('#idTableGSM').DataTable().ajax.reload(null, false);
     }, null );
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( type, selection )
  { $('#idModalDelTitre').text ( "Détruire le mapping ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer ce mapping ?"+
                                 "<hr>"+
                                 "<strong>"+selection.map_tech_id+":"+selection.map_tag +
                                 " <-> " + selection.tech_id + ":" + selection.acronyme + "</strong>" +
                                 "<br>" + selection.libelle
                                );
    $('#idModalDelValider').attr( "onclick",
                                  "Valider_GSM_Del('"+selection.map_tech_id+"','"+selection.map_tag+"')" );
    $('#idModalDel').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTableGSM').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DI ( )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'SMSG',
         tech_id    : $('#idModalEditSelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditSelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditGSMTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditGSMTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableGSM').DataTable().ajax.reload(null, false);
     }, null);
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function GSMMap_Update_Choix_Acronyme ()
  { Common_Updater_Choix_Acronyme ( 'idModalEdit', 'DI' );
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function GSMMap_Update_Choix_Tech_ID ( def_tech_id, def_acronyme )
  { Common_Updater_Choix_TechID ( 'idModalEdit', 'DI', def_tech_id, def_acronyme );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableGSM').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDI #idModalEditTitre').text ( "Editer MAP GSM" );
       GSMMap_Update_Choix_Tech_ID( selection.tech_id, selection.acronyme );
       $('#idModalEditDI #idModalEditGSMTag').val  ( selection.map_tag );
       $('#idModalEditDI #idModalEditGSMTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/process/smsg/list", null, function (Response)
        { $('#idModalEditGSMTechID').empty();
          $.each ( Response.gsms, function ( i, gsm )
           { $('#idModalEditGSMTechID').append("<option value='"+gsm.tech_id+"'"+
                                                (gsm.tech_id == selection.tech_id ? "selected" : "")+">"+
                                                 gsm.tech_id+ " (" +gsm.description+")</option>"); } );
        });
     }
    else
     { $('#idModalEditDI #idModalEditTitre').text ( "Ajouter un mapping DI" );
       $('#idModalEditRechercherTechID').val ( '' );
       $('#idModalEditDI #idModalEditGSMTag').val     ( '' );
       $('#idModalEditDI #idModalEditGSMTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
       Send_to_API ( "GET", "/api/process/smsg/list", null, function (Response)
        { $('#idModalEditGSMTechID').empty();
          $.each ( Response.gsms, function ( i, gsm )
           { $('#idModalEditGSMTechID').append("<option value='"+gsm.tech_id+"'>"+gsm.tech_id+ " (" +gsm.description+")</option>"); } );
        });
       GSMMap_Update_Choix_Tech_ID( null, null );
     }
    $('#idModalEditDI').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  {

    $('#idTableGSM').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "SMSG", "classe": "DI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"GSM TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"Texte SMS", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"BIT Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
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
