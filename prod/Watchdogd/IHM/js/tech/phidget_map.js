 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Phidget_Del ( type, id )
  { var json_request = JSON.stringify(
       { classe : type,
         id     : id,
       }
     );
    Send_to_API ( 'DELETE', "/api/process/phidget/map/del", json_request, function()
     { $('#idTablePhidgetMap'+type).DataTable().ajax.reload(null, false);
     }, null );
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del ( type, selection )
  { Show_modal_del ( "Détruire le mapping ?", "Etes-vous sur de vouloir supprimer ce mapping ?",
                     selection.hub_description+":"+selection.capteur +
                     " <-> " + selection.tech_id + ":" + selection.acronyme + " - " +
                     selection.libelle, "Valider_Phidget_Del('"+type+"','"+selection.id+"')" );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DI ( id )
  { table = $('#idTablePhidgetMapDI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_DO ( id )
  { table = $('#idTablePhidgetMapDO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "DO", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AI ( id )
  { table = $('#idTablePhidgetMapAI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Del_AO ( id )
  { table = $('#idTablePhidgetMapAO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    Show_Modal_Map_Del ( "AO", selection )
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Phidget_Edit_DI ( )
  { if ($('#idModalEditDISelectTechID').val() == null) return;
    if ($('#idModalEditDISelectAcronyme').val() == null) return;
    if (!isNum("idModalEditDIWagoTag")) return;
    var json_request = JSON.stringify(
       { classe     : 'DI',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditDISelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDISelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDIWagoTechID').val(),
         map_tag    : "DI"+$('#idModalEditDIWagoTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTablePhidgetMapDI').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idModalEditDI').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Phidget_Edit_DO ( )
  { if ($('#idModalEditDOSelectTechID').val() == null) return;
    if ($('#idModalEditDOSelectAcronyme').val() == null) return;
    if (!isNum("idModalEditDOWagoTag")) return;
    table = $('#idTablePhidgetMapDO').DataTable();
    var json_request = JSON.stringify(
       { classe     : 'DO',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditDOSelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDOSelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDOWagoTechID').val().toUpperCase(),
         map_tag    : "DO"+$('#idModalEditDOWagoTag').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTablePhidgetMapDO').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idModalEditDO').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Phidget_Edit_AI ( )
  { if ($('#idModalEditAISelectTechID').val() == null) return;
    if ($('#idModalEditAISelectAcronyme').val() == null) return;
    if (!isNum("idModalEditAIPort")) return;
    if (!isNum("idModalEditAIMin")) return;
    if (!isNum("idModalEditAIMax")) return;
    if (!isNum("idModalEditAIIntervalle")) return;
    if ($("#idModalEditAIPort").val()==null) return;
    if ($("#idModalEditAIUnite").val()==null) return;
    var json_request = JSON.stringify(
       { classe     : 'AI',
         capteur    : $('#idModalEditAICapteur').val(),
         hub_id     : $('#idModalEditAIHub').val(),
         port       : $('#idModalEditAIPort').val(),
         min        : $('#idModalEditAIMin').val(),
         max        : $('#idModalEditAIMax').val(),
         intervalle : $('#idModalEditAIIntervalle').val(),
         unite      : $('#idModalEditAIUnite').val(),
         tech_id    : $('#idModalEditAISelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditAISelectAcronyme').val().toUpperCase(),
         map_question_vocale: $('#idModalEditAIMapQuestionVoc').val(),
         map_reponse_vocale : $('#idModalEditAIMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/process/phidget/map/set", json_request, function ()
     { $('#idTablePhidgetMapAI').DataTable().ajax.reload(null, false);
       Process_reload ( null, "PHIDGET", false );
     });
    $('#idModalEditAI').modal("hide");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Phidget_Edit_AO ( )
  { if ($('#idModalEditAOSelectTechID').val() == null) return;
    if ($('#idModalEditAOSelectAcronyme').val() == null) return;
    if (!isNum("idModalEditAOWagoTag")) return;
    var json_request = JSON.stringify(
       { classe     : 'AO',
         thread     : 'MODBUS',
         tech_id    : $('#idModalEditAOSelectTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditAOSelectAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditAOWagoTechID').val().toUpperCase(),
         map_tag    : "AO"+$('#idModalEditAOWagoTag').val(),
         type       : $('#idModalEditAOType').val(),
         min        : $('#idModalEditAOMin').val(),
         max        : $('#idModalEditAOMax').val(),
         unite      : $('#idModalEditAOUnite').val(),
         map_question_vocale: $('#idModalEditAO #idModalEditMapQuestionVoc').val(),
         map_reponse_vocale : $('#idModalEditAO #idModalEditMapReponseVoc').val(),
       }
     );
    Send_to_API ( 'POST', "/api/map/set", json_request, function ()
     { $('#idTablePhidgetMapAO').DataTable().ajax.reload(null, false);
       Process_reload ( null, "MODBUS", false );
     });
    $('#idModalEditAO').modal("hide");
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function PhidgetMap_Update_Choix_Acronyme ( ids, classe )
  { Common_Updater_Choix_Acronyme ( ids, classe );
  }
/********************************************* Controle du saisie du modal ****************************************************/
 function PhidgetMap_Update_Choix_Tech_ID ( ids, classe, def_tech_id, def_acronyme )
  { Common_Updater_Choix_TechID ( ids, classe, def_tech_id, def_acronyme );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTablePhidgetMapDI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDITitre').text ( "Editer MAP DI - " + selection.map_tech_id + ":" + selection.map_tag );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditDI', 'DI', selection.tech_id, selection.acronyme );
       Select_from_api ( "idModalEditDIWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idModalEditDIWagoTag').val ( selection.map_tag.substring(2) );
       $('#idModalEditDIValider').attr( "onclick", "Valider_Phidget_Edit_DI()" );
     }
    else
     { $('#idModalEditDITitre').text ( "Ajouter un mapping DI" );
       Select_from_api ( "idModalEditDIWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idModalEditDIWagoTag').val ( "0" );
       $('#idModalEditDIRechercherTechID').val ( '' );
       $('#idModalEditDIValider').attr( "onclick", "Valider_Phidget_Edit_DI()" );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditDI', 'DI', null, null );
     }
    $('#idModalEditDI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DO ( id )
  { if (id>0)
     { table = $('#idTablePhidgetMapDO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDOTitre').text ( "Editer MAP DO - " + selection.map_tech_id + ":" + selection.map_tag );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditDO', 'DO', selection.tech_id, selection.acronyme );
       Select_from_api ( "idModalEditDOWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idModalEditDOWagoTag').val ( selection.map_tag.substring(2) );
       $('#idModalEditDOValider').attr( "onclick", "Valider_Phidget_Edit_DO()" );
     }
    else
     { $('#idModalEditDOTitre').text ( "Ajouter un mapping DO" );
       Select_from_api ( "idModalEditDOWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idModalEditDOWagoTag').val ( "0" );
       $('#idModalEditDORechercherTechID').val ( '' );
       $('#idModalEditDOValider').attr( "onclick", "Valider_Phidget_Edit_DO()" );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditDO', 'DO', null, null );
     }
    $('#idModalEditDO').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AI ( id )
  { $('#idModalEditAIUnite').on("change", function()
     { $('#idModalEditAIMinUnite').text($('#idModalEditAIUnite').val());
       $('#idModalEditAIMaxUnite').text($('#idModalEditAIUnite').val());
     });

    $('#idModalEditAICapteur').on("change", function()
     { switch($('#idModalEditAICapteur').val())
        { case "ADP1000-ORP": $('#idModalEditAIUnite').val("mV").prop('disabled', true).trigger('change');
                              $('#idModalEditAIMin').val("-400").prop('disabled', true);
                              $('#idModalEditAIMax').val("400").prop('disabled', true);
                              break;
          case "ADP1000-PH" : $('#idModalEditAIUnite').val("pH").prop('disabled', true).trigger('change');
                              $('#idModalEditAIMin').val("1").prop('disabled', true);
                              $('#idModalEditAIMax').val("14").prop('disabled', true);
                              break;
          default           : $('#idModalEditAIUnite').prop('disabled', false);
                              $('#idModalEditAIMin').prop('disabled', false);
                              $('#idModalEditAIMax').prop('disabled', false);
        }
     });

    if (id>0)
     { table = $('#idTablePhidgetMapAI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAITitre').text ( "Editer MAP AI - " + selection.hub_description + ":" + selection.capteur );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditAI', 'AI', selection.tech_id, selection.acronyme );
       Select_from_api ( "idModalEditAIHub", "/api/process/phidget/hub/list", null, "hubs", "id",
                         function (item) { return(item.hostname + " - " + item.serial+" - "+item.description) }, selection.hub_id );
       $('#idModalEditAIPort').val ( selection.port );
       $('#idModalEditAICapteur').val ( selection.capteur ).trigger('change');
       $('#idModalEditAIMin').val ( selection.min );
       $('#idModalEditAIMax').val ( selection.max );
       $('#idModalEditAIUnite').val ( selection.unite ).trigger('change');
       $('#idModalEditAIIntervalle').val ( selection.intervalle );
       $('#idModalEditAIMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAIMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAIValider').attr( "onclick", "Valider_Phidget_Edit_AI()" );
     }
    else
     { $('#idModalEditAITitre').text ( "Ajouter un MAP AI" );
       Select_from_api ( "idModalEditAIHub", "/api/process/phidget/hub/list", null, "hubs", "id",
                         function (item) { return(item.hostname + " - " + item.serial+" - "+item.description) }, null );
       $('#idModalEditAIPort').val ( '' );
       $('#idModalEditAICapteur').val("");
       $('#idModalEditAIRechercherTechID').val ( '' );
       $('#idModalEditAIMapQuestionVoc').val ( '' );
       $('#idModalEditAIMapReponseVoc').val ( '' );
       $('#idModalEditAIIntervalle').val ( "5" );
       $('#idModalEditAIValider').attr( "onclick", "Valider_Phidget_Edit_AI()" );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditAI', 'AI', null, null );
     }

    $('#idModalEditAI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AO ( id )
  { if (id>0)
     { table = $('#idTablePhidgetMapAO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAOTitre').text ( "Editer MAP AO - " + selection.map_tech_id + ":" + selection.map_tag );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditAO', 'AO', selection.tech_id, selection.acronyme );
       Select_from_api ( "idModalEditAOWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, selection.map_tech_id );
       $('#idModalEditAOWagoTag').val ( selection.map_tag.substring(2) );
       $('#idModalEditAOType').val ( selection.type );
       $('#idModalEditAOMin').val ( selection.min );
       $('#idModalEditAOMax').val ( selection.max );
       $('#idModalEditAOUnite').val ( selection.unite );
       $('#idModalEditAOMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAOMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAOValider').attr( "onclick", "Valider_Phidget_Edit_AO()" );
     }
    else
     { $('#idModalEditAOTitre').text ( "Ajouter un MAP AO" );
       Select_from_api ( "idModalEditAOWagoTechID", "/api/process/phidget/list", null, "modules", "tech_id",
                         function (item) { return(item.tech_id) }, null );
       $('#idModalEditAOWagoTag').val ( "0" );
       $('#idModalEditAORechercherTechID').val ( '' );
       $('#idModalEditAOType').val ( 0 );
       $('#idModalEditAOMapQuestionVoc').val ( '' );
       $('#idModalEditAOMapReponseVoc').val ( '' );
       $('#idModalEditAOValider').attr( "onclick", "Valider_Phidget_Edit_AO()" );
       PhidgetMap_Update_Choix_Tech_ID( 'idModalEditAO', 'AO', null, null );
     }
    $('#idModalEditAO').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/phidget/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
/*    $('#idTablePhidgetMapDI').DataTable(
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
         responsive: true,
       }
     );

    $('#idTablePhidgetMapDO').DataTable(
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
         responsive: true,
       }
     );
*/
    $('#idTablePhidgetMapAI').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/process/phidget/map/list",	type : "GET", dataSrc: "mappings", data: { "classe": "AI" },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Hub", "className": "align-middle text-center",
              "render": function (item)
                { return( item.hub_hostname+" - "+item.hub_description ); }
            },
            { "data": "port", "title":"Port", "className": "align-middle text-center" },
            { "data": "intervalle", "title":"Intervalle", "className": "align-middle text-center" },
            { "data": null, "title":"Capteur", "className": "align-middle text-center",
              "render": function (item)
                { return( item.capteur+" - "+item.classe ); }
            },
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
/*
    $('#idTablePhidgetMapAO').DataTable(
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
         responsive: true,
       }
     );
*/
    $('#idTabEntreeTor').tab('show');
  }