 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Del ( type, map_tech_id, map_tag )
  { var json_request = JSON.stringify(
       { classe     : type,
         map_tech_id: map_tech_id,
         map_tag    : map_tag
       }
     );
    Send_to_API ( 'DELETE', "/api/map/del", json_request, function()
     { $('#idTableMODBUS_'+type).DataTable().ajax.reload(null, false);
       $('#idToastStatus').toast('show');
     }, null );
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( type, selection )
  { Show_modal_del ( "Détruire le mapping ?", "Etes-vous sur de vouloir supprimer ce mapping ?",
                     selection.map_tech_id+":"+selection.map_tag +
                     " <-> " + selection.tech_id + ":" + selection.acronyme + " - " +
                     selection.libelle,
                     function () { Valider_Modbus_Del(type,selection.map_tech_id,selection.map_tag); } );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTableMODBUS_DI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DO ( id )
  { table = $('#idTableMODBUS_DO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DO", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AI ( id )
  { table = $('#idTableMODBUS_AI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AO ( id )
  { table = $('#idTableMODBUS_AO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AO", selection )
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit_DI ( )
  { if ($('#idMODBUSEditDISelectTechID').val() == null) return;
    if ($('#idMODBUSEditDISelectAcronyme').val() == null) return;
    if (!isNum("idMODBUSEditDIWagoTag")) return;
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'MODBUS',
         tech_id    : $('#idMODBUSEditDISelectTechID').val().toUpperCase(),
         acronyme   : $('#idMODBUSEditDISelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idMODBUSEditDIWagoTechID').val(),
         map_tag    : "DI"+$('#idMODBUSEditDIWagoTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableMODBUS_DI').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idMODBUSEditDI').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit_DO ( )
  { if ($('#idMODBUSEditDOSelectTechID').val() == null) return;
    if ($('#idMODBUSEditDOSelectAcronyme').val() == null) return;
    if (!isNum("idMODBUSEditDOWagoTag")) return;
    table = $('#idTableMODBUS_DO').DataTable();
    var json_request = JSON.stringify(
       { classe     : 'DO',
         thread     : 'MODBUS',
         tech_id    : $('#idMODBUSEditDOSelectTechID').val().toUpperCase(),
         acronyme   : $('#idMODBUSEditDOSelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idMODBUSEditDOWagoTechID').val().toUpperCase(),
         map_tag    : "DO"+$('#idMODBUSEditDOWagoTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableMODBUS_DO').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idMODBUSEditDO').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit_AI ( )
  { if ($('#idMODBUSEditAISelectTechID').val() == null) return;
    if ($('#idMODBUSEditAISelectAcronyme').val() == null) return;
    if (!isNum("idMODBUSEditAIWagoTag")) return;
    var json_request = JSON.stringify(
       { classe     : 'AI',
         thread     : 'MODBUS',
         tech_id    : $('#idMODBUSEditAISelectTechID').val().toUpperCase(),
         acronyme   : $('#idMODBUSEditAISelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idMODBUSEditAIWagoTechID').val().toUpperCase(),
         map_tag    : "AI"+$('#idMODBUSEditAIWagoTag').val(),
         type       : parseInt($('#idMODBUSEditAIType').val()),
         min        : parseInt($('#idMODBUSEditAIMin').val()),
         max        : parseInt($('#idMODBUSEditAIMax').val()),
         unite      : $('#idMODBUSEditAIUnite').val(),
         map_question_vocale: $('#idMODBUSEditAI #idMODBUSEditMapQuestionVoc').val(),
         map_reponse_vocale : $('#idMODBUSEditAI #idMODBUSEditMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableMODBUS_AI').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idMODBUSEditAI').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Edit_AO ( )
  { if ($('#idMODBUSEditAOSelectTechID').val() == null) return;
    if ($('#idMODBUSEditAOSelectAcronyme').val() == null) return;
    if (!isNum("idMODBUSEditAOWagoTag")) return;
    var json_request = JSON.stringify(
       { classe     : 'AO',
         thread     : 'MODBUS',
         tech_id    : $('#idMODBUSEditAOSelectTechID').val().toUpperCase(),
         acronyme   : $('#idMODBUSEditAOSelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idMODBUSEditAOWagoTechID').val().toUpperCase(),
         map_tag    : "AO"+$('#idMODBUSEditAOWagoTag').val(),
         type       : parseInt($('#idMODBUSEditAOType').val()),
         min        : parseInt($('#idMODBUSEditAOMin').val()),
         max        : parseInt($('#idMODBUSEditAOMax').val()),
         unite      : $('#idMODBUSEditAOUnite').val(),
         map_question_vocale: $('#idMODBUSEditAO #idMODBUSEditMapQuestionVoc').val(),
         map_reponse_vocale : $('#idMODBUSEditAO #idMODBUSEditMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableMODBUS_AO').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idMODBUSEditAO').modal("hide");
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function ModbusMap_Update_Choix_Acronyme ( ids, classe )
  { Common_Updater_Choix_Acronyme ( ids, classe );
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function ModbusMap_Update_Choix_Tech_ID ( ids, classe, def_tech_id, def_acronyme )
  { Common_Updater_Choix_TechID ( ids, classe, def_tech_id, def_acronyme );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableMODBUS_DI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idMODBUSEditDITitre').text ( "Editer MAP DI - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditDI', 'DI', selection.tech_id, selection.acronyme );
       Select_from_api ( "idMODBUSEditDIWagoTechID", "/api/process/modbus/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idMODBUSEditDIWagoTag').val ( selection.map_tag.substring(2) );
       $('#idMODBUSEditDIValider').attr( "onclick", "Valider_Modbus_Edit_DI()" );
     }
    else
     { $('#idMODBUSEditDITitre').text ( "Ajouter un mapping DI" );
       Select_from_api ( "idMODBUSEditDIWagoTechID", "/api/process/config", "name=modbus", "config", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idMODBUSEditDIWagoTag').val ( "0" );
       $('#idMODBUSEditDIRechercherTechID').val ( '' );
       $('#idMODBUSEditDIValider').attr( "onclick", "Valider_Modbus_Edit_DI()" );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditDI', 'DI', null, null );
     }
    $('#idMODBUSEditDI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit_DO ( id )
  { if (id>0)
     { table = $('#idTableMODBUS_DO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idMODBUSEditDOTitre').text ( "Editer MAP DO - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditDO', 'DO', selection.tech_id, selection.acronyme );
       Select_from_api ( "idMODBUSEditDOWagoTechID", "/api/process/modbus/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idMODBUSEditDOWagoTag').val ( selection.map_tag.substring(2) );
       $('#idMODBUSEditDOValider').attr( "onclick", "Valider_Modbus_Edit_DO()" );
     }
    else
     { $('#idMODBUSEditDOTitre').text ( "Ajouter un mapping DO" );
       Select_from_api ( "idMODBUSEditDOWagoTechID", "/api/process/config", "name=modbus", "config", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idMODBUSEditDOWagoTag').val ( "0" );
       $('#idMODBUSEditDORechercherTechID').val ( '' );
       $('#idMODBUSEditDOValider').attr( "onclick", "Valider_Modbus_Edit_DO()" );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditDO', 'DO', null, null );
     }
    $('#idMODBUSEditDO').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit_AI ( id )
  { if (id>0)
     { table = $('#idTableMODBUS_AI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idMODBUSEditAITitre').text ( "Editer MAP AI - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditAI', 'AI', selection.tech_id, selection.acronyme );
       Select_from_api ( "idMODBUSEditAIWagoTechID", "/api/process/modbus/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idMODBUSEditAIWagoTag').val ( selection.map_tag.substring(2) );
       $('#idMODBUSEditAIType').val ( selection.type );
       $('#idMODBUSEditAIMin').val ( selection.min );
       $('#idMODBUSEditAIMax').val ( selection.max );
       $('#idMODBUSEditAIUnite').val ( selection.unite );
       $('#idMODBUSEditAIMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idMODBUSEditAIMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idMODBUSEditAIValider').attr( "onclick", "Valider_Modbus_Edit_AI()" );
     }
    else
     { $('#idMODBUSEditAITitre').text ( "Ajouter un MAP AI" );
       Select_from_api ( "idMODBUSEditAIWagoTechID", "/api/process/config", "name=modbus", "config", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idMODBUSEditAIWagoTag').val ( "0" );
       $('#idMODBUSEditAIRechercherTechID').val ( '' );
       $('#idMODBUSEditAIType').val ( 0 );
       $('#idMODBUSEditAIMapQuestionVoc').val ( '' );
       $('#idMODBUSEditAIMapReponseVoc').val ( '' );
       $('#idMODBUSEditAIValider').attr( "onclick", "Valider_Modbus_Edit_AI()" );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditAI', 'AI', null, null );
     }
    $('#idMODBUSEditAI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit_AO ( id )
  { if (id>0)
     { table = $('#idTableMODBUS_AO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idMODBUSEditAOTitre').text ( "Editer MAP AO - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditAO', 'AO', selection.tech_id, selection.acronyme );
       Select_from_api ( "idMODBUSEditAOWagoTechID", "/api/process/modbus/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idMODBUSEditAOWagoTag').val ( selection.map_tag.substring(2) );
       $('#idMODBUSEditAOType').val ( selection.type );
       $('#idMODBUSEditAOMin').val ( selection.min );
       $('#idMODBUSEditAOMax').val ( selection.max );
       $('#idMODBUSEditAOUnite').val ( selection.unite );
       $('#idMODBUSEditAOMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idMODBUSEditAOMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idMODBUSEditAOValider').attr( "onclick", "Valider_Modbus_Edit_AO()" );
     }
    else
     { $('#idMODBUSEditAOTitre').text ( "Ajouter un MAP AO" );
       Select_from_api ( "idMODBUSEditAOWagoTechID", "/api/process/config", "name=modbus", "config", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idMODBUSEditAOWagoTag').val ( "0" );
       $('#idMODBUSEditAORechercherTechID').val ( '' );
       $('#idMODBUSEditAOType').val ( 0 );
       $('#idMODBUSEditAOMapQuestionVoc').val ( '' );
       $('#idMODBUSEditAOMapReponseVoc').val ( '' );
       $('#idMODBUSEditAOValider').attr( "onclick", "Valider_Modbus_Edit_AO()" );
       ModbusMap_Update_Choix_Tech_ID( 'idMODBUSEditAO', 'AO', null, null );
     }
    $('#idMODBUSEditAO').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMODBUS_DI').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "MODBUS", "classe": "DI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"WAGO TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"WAGO I/O", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "<->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"BIT Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "MODBUS_Edit_DI", item.id, "pen", null );
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

    $('#idTableMODBUS_DO').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "MODBUS", "classe": "DO" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"WAGO TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"WAGO I/O", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "<->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"BIT Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "MODBUS_Edit_DO", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet objet", "Show_Modal_Map_Del_DO", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            },
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableMODBUS_AI').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "MODBUS", "classe": "AI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"WAGO TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"WAGO I/O", "className": "align-middle text-center" },
             { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "<->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": null, "title":"BIT Acronyme", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/courbe/"+item.tech_id+"/"+item.acronyme+"/HOUR", "Voir le graphe", item.acronyme ) ); }
            },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Echange vocaux", "className": "align-left text-center",
              "render": function (item)
                { return( "Question : "+item.map_question_vocale+"<br>Réponse:"+item.map_reponse_vocale ); }
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "MODBUS_Edit_AI", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet objet", "Show_Modal_Map_Del_AI", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            },
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableMODBUS_AO').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/map/list",	type : "GET", dataSrc: "mappings", data: { "thread": "MODBUS", "classe": "AO" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "map_tech_id", "title":"WAGO TechID", "className": "align-middle text-center" },
            { "data": "map_tag", "title":"WAGO I/O", "className": "align-middle text-center" },
            { "data": null, "title":"Map", "className": "align-middle text-center",
              "render": function (item)
                { return( "<->" ); }
            },
            { "data": null, "title":"BIT Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
            },
            { "data": "acronyme", "title":"BIT Acronyme", "className": "align-middle text-center" },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "MODBUS_Edit_AO", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet objet", "Show_Modal_Map_Del_AO", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            },
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTabEntreeTor').tab('show');
  }
