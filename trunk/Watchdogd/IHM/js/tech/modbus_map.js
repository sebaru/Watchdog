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
     { $('#idTableModbusMap'+type).DataTable().ajax.reload(null, false);
       $('#idToastStatus').toast('show');
     }, null );
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( type, selection )
  { $('#idModalModbusDelTitre').text ( "Détruire le mapping ?" );
    $('#idModalModbusDelMessage').html("Etes-vous sur de vouloir supprimer ce mapping ?"+
                                    "<hr>"+
                                    "<strong>"+selection.map_tech_id+":"+selection.map_tag +
                                    " <-> " + selection.tech_id + ":" + selection.acronyme + "</strong>" +
                                    "<br>" + selection.libelle
                                   );
    $('#idModalModbusDelValider').attr( "onclick",
                                        "Valider_Modbus_Del('"+type+"','"+selection.map_tech_id+"','"+selection.map_tag+"')" );
    $('#idModalModbusDel').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTableModbusMapDI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DO ( id )
  { table = $('#idTableModbusMapDO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DO", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AI ( id )
  { table = $('#idTableModbusMapAI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AO ( id )
  { table = $('#idTableModbusMapAO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AO", selection )
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DI ( )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditDI #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDI #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDI #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditDI #idModalEditWagoTag').val().toUpperCase(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableModbusMapDI').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_DO ( )
  { if ($('#idModalEditDO #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDO').modal("hide");
    table = $('#idTableModbusMapDO').DataTable();
    var json_request = JSON.stringify(
       { classe     : 'DO',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditDO #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDO #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDO #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditDO #idModalEditWagoTag').val().toUpperCase(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableModbusMapDO').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_AI ( )
  { if ($('#idModalEditAI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditAI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'AI',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditAI #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditAI #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditAI #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditAI #idModalEditWagoTag').val().toUpperCase(),
         type       : $('#idModalEditAI #idModalEditType').val(),
         min        : $('#idModalEditAI #idModalEditMin').val(),
         max        : $('#idModalEditAI #idModalEditMax').val(),
         unite      : $('#idModalEditAI #idModalEditUnite').val(),
         map_question_vocale: $('#idModalEditAI #idModalEditMapQuestionVoc').val(),
         map_reponse_vocale : $('#idModalEditAI #idModalEditMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableModbusMapAI').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_AO ( )
  { if ($('#idModalEditAO #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditAI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'AO',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditAO #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditAO #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditAO #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditAO #idModalEditWagoTag').val().toUpperCase(),
         type       : $('#idModalEditAO #idModalEditType').val(),
         min        : $('#idModalEditAO #idModalEditMin').val(),
         max        : $('#idModalEditAO #idModalEditMax').val(),
         unite      : $('#idModalEditAO #idModalEditUnite').val(),
         map_question_vocale: $('#idModalEditAO #idModalEditMapQuestionVoc').val(),
         map_reponse_vocale : $('#idModalEditAO #idModalEditMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTableModbusMapAI').DataTable().ajax.reload(null, false);
     });
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
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableModbusMapDI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDITitre').text ( "Editer MAP DI - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditDI', 'DI', selection.tech_id, selection.acronyme );
       $('#idModalEditDIWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditDIWagoTechID').attr ( "readonly", true );
       $('#idModalEditDIWagoTag').val     ( selection.map_tag );
       $('#idModalEditDIWagoTag').attr ( "readonly", true );
       $('#idModalEditDIValider').attr( "onclick", "Valider_Edit_DI()" );
     }
    else
     { $('#idModalEditDITitre').text ( "Ajouter un mapping DI" );
       $('#idModalEditDIRechercherTechID').val ( '' );
       $('#idModalEditDIWagoTechID').val  ( '' );
       $('#idModalEditDIWagoTechID').attr ( "readonly", false );
       $('#idModalEditDIWagoTag').val     ( '' );
       $('#idModalEditDIWagoTag').attr ( "readonly", false );
       $('#idModalEditDIValider').attr( "onclick", "Valider_Edit_DI()" );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditDI', 'DI', null, null );
     }
    $('#idModalEditDI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DO ( id )
  { if (id>0)
     { table = $('#idTableModbusMapDO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDOTitre').text ( "Editer MAP DO - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditDO', 'DO', selection.tech_id, selection.acronyme );
       $('#idModalEditDOWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditDOWagoTechID').attr ( "readonly", true );
       $('#idModalEditDOWagoTag').val     ( selection.map_tag );
       $('#idModalEditDOWagoTag').attr ( "readonly", true );
       $('#idModalEditDOValider').attr( "onclick", "Valider_Edit_DO()" );
     }
    else
     { $('#idModalEditDOTitre').text ( "Ajouter un mapping DO" );
       $('#idModalEditDORechercherTechID').val ( '' );
       $('#idModalEditDOWagoTechID').val  ( '' );
       $('#idModalEditDOWagoTechID').attr ( "readonly", false );
       $('#idModalEditDOWagoTag').val     ( '' );
       $('#idModalEditDOWagoTag').attr ( "readonly", false );
       $('#idModalEditDOValider').attr( "onclick", "Valider_Edit_DO()" );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditDO', 'DO', null, null );
     }
    $('#idModalEditDO').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AI ( id )
  { if (id>0)
     { table = $('#idTableModbusMapAI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAITitre').text ( "Editer MAP AI - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditAI', 'AI', selection.tech_id, selection.acronyme );
       $('#idModalEditAIWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditAIWagoTechID').attr ( "readonly", true );
       $('#idModalEditAIWagoTag').val     ( selection.map_tag );
       $('#idModalEditAIWagoTag').attr ( "readonly", true );
       $('#idModalEditAIType').val ( selection.type );
       $('#idModalEditAIMin').val ( selection.min );
       $('#idModalEditAIMax').val ( selection.max );
       $('#idModalEditAIUnite').val ( selection.unite );
       $('#idModalEditAIMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAIMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAIValider').attr( "onclick", "Valider_Edit_AI()" );
     }
    else
     { $('#idModalEditAITitre').text ( "Ajouter un MAP AI" );
       $('#idModalEditAIWagoTechID').val  ( '' );
       $('#idModalEditAIWagoTechID').attr ( "readonly", false );
       $('#idModalEditAIWagoTag').val     ( '' );
       $('#idModalEditAIWagoTag').attr ( "readonly", false );
       $('#idModalEditAIType').val ( 0 );
       $('#idModalEditAIMapQuestionVoc').val ( '' );
       $('#idModalEditAIMapReponseVoc').val ( '' );
       $('#idModalEditAIValider').attr( "onclick", "Valider_Edit_AI()" );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditAI', 'AI', null, null );
     }
    $('#idModalEditAI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AO ( id )
  { if (id>0)
     { table = $('#idTableModbusMapAO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAOTitre').text ( "Editer MAP AO - " + selection.map_tech_id + ":" + selection.map_tag );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditAO', 'AO', selection.tech_id, selection.acronyme );
       $('#idModalEditAOWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditAOWagoTechID').attr ( "readonly", true );
       $('#idModalEditAOWagoTag').val     ( selection.map_tag );
       $('#idModalEditAOWagoTag').attr ( "readonly", true );
       $('#idModalEditAOType').val ( selection.type );
       $('#idModalEditAOMin').val ( selection.min );
       $('#idModalEditAOMax').val ( selection.max );
       $('#idModalEditAOUnite').val ( selection.unite );
       $('#idModalEditAOMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAOMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAOValider').attr( "onclick", "Valider_Edit_AO()" );
     }
    else
     { $('#idModalEditAOTitre').text ( "Ajouter un MAP AO" );
       $('#idModalEditAOTechID').val ( '' );
       $('#idModalEditAOAcronyme').val ( '' );
       $('#idModalEditAOWagoTechID').val  ( '' );
       $('#idModalEditAOWagoTechID').attr ( "readonly", false );
       $('#idModalEditAOWagoTag').val     ( '' );
       $('#idModalEditAOWagoTag').attr ( "readonly", false );
       $('#idModalEditAOType').val ( 0 );
       $('#idModalEditAOMapQuestionVoc').val ( '' );
       $('#idModalEditAOMapReponseVoc').val ( '' );
       $('#idModalEditAOValider').attr( "onclick", "Valider_Edit_AO()" );
       ModbusMap_Update_Choix_Tech_ID( 'idModalEditAO', 'AO', null, null );
     }
    $('#idModalEditAO').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Add ( )
  { var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/process/modbus/add" );
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { tech_id :    $('#idModalModbusEditTechID').val(),
         hostname:    $('#idModalModbusEditHostname').val(),
         description: $('#idModalModbusEditDescription').val(),
         watchdog:    $('#idModalModbusEditWatchdog').val(),
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableModbusMap').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Modbus_Add ()
  { $('#idModalModbusEditTitre').text ( "Ajouter un composant WAGO" );
    $('#idModalModbusEditTechID').val ( "" );
    $('#idModalModbusEditTechID').attr ( "readonly", false );
    $('#idModalModbusEditHostname').val ( "" );
    $('#idModalModbusEditDescription').val( "" );
    $('#idModalModbusEditWatchdog').val( "" );
    $('#idModalModbusEditValider').attr( "onclick", "Valider_Modbus_Add()" );
    $('#idModalModbusEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/modbus/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
    $('#idTableModbusMapDI').DataTable(
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

    $('#idTableModbusMapDO').DataTable(
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "Show_Modal_Map_Edit_DO", item.id, "pen", null );
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

    $('#idTableModbusMapAI').DataTable(
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
                { return( Lien ( "/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR", "Voir le graphe", item.acronyme ) ); }
            },
            { "data": "libelle", "title":"BIT Libelle", "className": "align-middle text-center" },
            { "data": null, "title":"Echange vocaux", "className": "align-left text-center",
              "render": function (item)
                { return( "Question : "+item.map_question_vocale+"<br>Réponse:"+item.map_reponse_vocale ); }
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "Show_Modal_Map_Edit_AI", item.id, "pen", null );
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

    $('#idTableModbusMapAO').DataTable(
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet objet", "Show_Modal_Map_Edit_AO", item.id, "pen", null );
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
