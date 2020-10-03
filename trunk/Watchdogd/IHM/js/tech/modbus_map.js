 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Del ( type, map_tech_id, map_tag )
  { var xhr = new XMLHttpRequest;
    xhr.open('DELETE', "/api/process/modbus/map/del" );
    var json_request = JSON.stringify(
       { classe     : type,
         map_tech_id: map_tech_id,
         map_tag    : map_tag
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableModbusMap'+type).DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
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
         tech_id    : $('#idModalEditDI #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDI #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDI #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditDI #idModalEditWagoTag').val().toUpperCase(),
       }
     );
    Send_to_API ( 'POST', "/api/process/modbus/map/set", json_request, function ()
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
         tech_id    : $('#idModalEditDO #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditDO #idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: $('#idModalEditDO #idModalEditWagoTechID').val().toUpperCase(),
         map_tag    : $('#idModalEditDO #idModalEditWagoTag').val().toUpperCase(),
       }
     );
    Send_to_API ( 'POST', "/api/process/modbus/map/set", json_request, function ()
     { $('#idTableModbusMapDO').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_AI ( )
  { if ($('#idModalEditAI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditAI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'AI',
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
    Send_to_API ( 'POST', "/api/process/modbus/map/set", json_request, function ()
     { $('#idTableModbusMapAI').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Edit_AO ( )
  { if ($('#idModalEditAO #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditAI').modal("hide");
    var json_request = JSON.stringify(
       { classe     : 'AO',
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
    Send_to_API ( 'POST', "/api/process/modbus/map/set", json_request, function ()
     { $('#idTableModbusMapAI').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Modal_Edit_Input_Changed ( target )
  { const Ascii_charset = RegExp(/^[a-zA-Z0-9][a-zA-Z0-9_]*$/);

    if ($('#'+target+' #idModalEditTechID').val().length==0)
     { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#'+target+' #idModalEditTechIDPropose').text("Aucun match");
       $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#'+target+' #idModalEditAcronyme').prop("disabled", true);
       $('#'+target+' #idModalEditAcronyme').val("");
       $('#'+target+' #idModalEditAcronymePropose').text("Aucun match");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    if (!Ascii_charset.test($('#'+target+' #idModalEditTechID').val()))
     { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#'+target+' #idModalEditTechIDPropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    if ($('#'+target+' #idModalEditAcronyme').val().length && !Ascii_charset.test($('#'+target+' #idModalEditAcronyme').val()))
     { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#'+target+' #idModalEditAcronymePropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       $('#'+target+' #idModalEditValider').addClass("disabled");
       return;
     }

    var json_request = JSON.stringify(
       { tech_id    : $('#'+target+' #idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#'+target+' #idModalEditAcronyme').val().toUpperCase(),
       }
     );
    Send_to_API ( "PUT", "/api/mnemos/validate", json_request, function (Response)
     { var tech_id_found=false, tech_id_propose="";
       var acronyme_found=false, acronyme_propose="";

       for (var i = 0; i < Response.nbr_tech_ids_found; i++)
        { tech_id_propose=tech_id_propose + Response.tech_ids_found[i].tech_id+" ";
          if (Response.tech_ids_found[i].tech_id == $('#'+target+' #idModalEditTechID').val().toUpperCase()) tech_id_found=true;
        }
       if (tech_id_found==true)
        { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning");
          $('#'+target+' #idModalEditTechIDPropose').text("Match !");
          $('#'+target+' #idModalEditAcronyme').prop("disabled", false);
        }
       else
        { $('#'+target+' #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
          $('#'+target+' #idModalEditTechIDPropose').text("Choix: " + tech_id_propose);
          $('#'+target+' #idModalEditAcronyme').prop("disabled", true);
          $('#'+target+' #idModalEditAcronyme').val("");
        }

       for (var i = 0; i < Response.acronymes_found.length; i++)
        { acronyme_propose=acronyme_propose + Response.acronymes_found[i].acronyme+" ";
          if (Response.acronymes_found[i].acronyme == $('#'+target+' #idModalEditAcronyme').val().toUpperCase()) acronyme_found=true;
        }
       if (acronyme_found==true)
        { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning");
          $('#'+target+' #idModalEditAcronymePropose').text("Match !");
        }
       else
        { $('#'+target+' #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
          $('#'+target+' #idModalEditAcronymePropose').text("Choix: " + acronyme_propose);
        }
       if (tech_id_found==true && acronyme_found==true)
        { $('#'+target+' #idModalEditValider').removeClass("disabled"); }
       else
        { $('#'+target+' #idModalEditValider').addClass("disabled");    }
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DI ( id )
  { if (id>0)
     { table = $('#idTableModbusMapDI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDI #idModalEditTitre').text ( "Editer MAP DI - " + selection.map_tech_id + ":" + selection.map_tag );
       $('#idModalEditDI #idModalEditTechID').val ( selection.tech_id );
       $('#idModalEditDI #idModalEditAcronyme').val ( selection.acronyme );
       $('#idModalEditDI #idModalEditWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditDI #idModalEditWagoTechID').attr ( "readonly", true );
       $('#idModalEditDI #idModalEditWagoTag').val     ( selection.map_tag );
       $('#idModalEditDI #idModalEditWagoTag').attr ( "readonly", true );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
     }
    else
     { $('#idModalEditDI #idModalEditTitre').text ( "Ajouter un mapping DI" );
       $('#idModalEditDI #idModalEditTechID').val ( '' );
       $('#idModalEditDI #idModalEditAcronyme').val ( '' );
       $('#idModalEditDI #idModalEditWagoTechID').val  ( '' );
       $('#idModalEditDI #idModalEditWagoTechID').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditWagoTag').val     ( '' );
       $('#idModalEditDI #idModalEditWagoTag').attr ( "readonly", false );
       $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI()" );
     }
    Modal_Edit_Input_Changed('idModalEditDI');
    $('#idModalEditDI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DO ( id )
  { if (id>0)
     { table = $('#idTableModbusMapDO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditDO #idModalEditTitre').text ( "Editer MAP DO - " + selection.map_tech_id + ":" + selection.map_tag );
       $('#idModalEditDO #idModalEditTechID').val ( selection.tech_id );
       $('#idModalEditDO #idModalEditAcronyme').val ( selection.acronyme );
       $('#idModalEditDO #idModalEditWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditDO #idModalEditWagoTechID').attr ( "readonly", true );
       $('#idModalEditDO #idModalEditWagoTag').val     ( selection.map_tag );
       $('#idModalEditDO #idModalEditWagoTag').attr ( "readonly", true );
       $('#idModalEditDO #idModalEditValider').attr( "onclick", "Valider_Edit_DO()" );
     }
    else
     { $('#idModalEditDO #idModalEditTitre').text ( "Ajouter un mapping DO" );
       $('#idModalEditDO #idModalEditTechID').val ( '' );
       $('#idModalEditDO #idModalEditAcronyme').val ( '' );
       $('#idModalEditDO #idModalEditWagoTechID').val  ( '' );
       $('#idModalEditDO #idModalEditWagoTechID').attr ( "readonly", false );
       $('#idModalEditDO #idModalEditWagoTag').val     ( '' );
       $('#idModalEditDO #idModalEditWagoTag').attr ( "readonly", false );
       $('#idModalEditDO #idModalEditValider').attr( "onclick", "Valider_Edit_DO()" );
     }
    Modal_Edit_Input_Changed('idModalEditDO');
    $('#idModalEditDO').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AI ( id )
  { if (id>0)
     { table = $('#idTableModbusMapAI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAI #idModalEditTitre').text ( "Editer MAP AI - " + selection.map_tech_id + ":" + selection.map_tag );
       $('#idModalEditAI #idModalEditTechID').val ( selection.tech_id );
       $('#idModalEditAI #idModalEditAcronyme').val ( selection.acronyme );
       $('#idModalEditAI #idModalEditWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditAI #idModalEditWagoTechID').attr ( "readonly", true );
       $('#idModalEditAI #idModalEditWagoTag').val     ( selection.map_tag );
       $('#idModalEditAI #idModalEditWagoTag').attr ( "readonly", true );
       $('#idModalEditAI #idModalEditType').val ( selection.type );
       $('#idModalEditAI #idModalEditMin').val ( selection.min );
       $('#idModalEditAI #idModalEditMax').val ( selection.max );
       $('#idModalEditAI #idModalEditUnite').val ( selection.unite );
       $('#idModalEditAI #idModalEditMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAI #idModalEditMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAI #idModalEditValider').attr( "onclick", "Valider_Edit_AI()" );
     }
    else
     { $('#idModalEditAI #idModalEditTitre').text ( "Ajouter un MAP AI" );
       $('#idModalEditAI #idModalEditTechID').val ( '' );
       $('#idModalEditAI #idModalEditAcronyme').val ( '' );
       $('#idModalEditAI #idModalEditWagoTechID').val  ( '' );
       $('#idModalEditAI #idModalEditWagoTechID').attr ( "readonly", false );
       $('#idModalEditAI #idModalEditWagoTag').val     ( '' );
       $('#idModalEditAI #idModalEditWagoTag').attr ( "readonly", false );
       $('#idModalEditAI #idModalEditType').val ( 0 );
       $('#idModalEditAI #idModalEditMapQuestionVoc').val ( '' );
       $('#idModalEditAI #idModalEditMapReponseVoc').val ( '' );
       $('#idModalEditAI #idModalEditValider').attr( "onclick", "Valider_Edit_AI()" );
     }
    Modal_Edit_Input_Changed('idModalEditAI');
    $('#idModalEditAI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AO ( id )
  { if (id>0)
     { table = $('#idTableModbusMapAO').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       $('#idModalEditAI #idModalEditTitre').text ( "Editer MAP AO - " + selection.map_tech_id + ":" + selection.map_tag );
       $('#idModalEditAI #idModalEditTechID').val ( selection.tech_id );
       $('#idModalEditAI #idModalEditAcronyme').val ( selection.acronyme );
       $('#idModalEditAI #idModalEditWagoTechID').val  ( selection.map_tech_id );
       $('#idModalEditAI #idModalEditWagoTechID').attr ( "readonly", true );
       $('#idModalEditAI #idModalEditWagoTag').val     ( selection.map_tag );
       $('#idModalEditAI #idModalEditWagoTag').attr ( "readonly", true );
       $('#idModalEditAI #idModalEditType').val ( selection.type );
       $('#idModalEditAI #idModalEditMin').val ( selection.min );
       $('#idModalEditAI #idModalEditMax').val ( selection.max );
       $('#idModalEditAI #idModalEditUnite').val ( selection.unite );
       $('#idModalEditAI #idModalEditMapQuestionVoc').val ( selection.map_question_vocale );
       $('#idModalEditAI #idModalEditMapReponseVoc').val ( selection.map_reponse_vocale );
       $('#idModalEditAI #idModalEditValider').attr( "onclick", "Valider_Edit_AO()" );
     }
    else
     { $('#idModalEditAI #idModalEditTitre').text ( "Ajouter un MAP AI" );
       $('#idModalEditAI #idModalEditTechID').val ( '' );
       $('#idModalEditAI #idModalEditAcronyme').val ( '' );
       $('#idModalEditAI #idModalEditWagoTechID').val  ( '' );
       $('#idModalEditAI #idModalEditWagoTechID').attr ( "readonly", false );
       $('#idModalEditAI #idModalEditWagoTag').val     ( '' );
       $('#idModalEditAI #idModalEditWagoTag').attr ( "readonly", false );
       $('#idModalEditAI #idModalEditType').val ( 0 );
       $('#idModalEditAI #idModalEditMapQuestionVoc').val ( '' );
       $('#idModalEditAI #idModalEditMapReponseVoc').val ( '' );
       $('#idModalEditAI #idModalEditValider').attr( "onclick", "Valider_Edit_AO()" );
     }
    Modal_Edit_Input_Changed('idModalEditAO');
    $('#idModalEditAI').modal("show");
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
         ajax: {	url : "/api/process/modbus/map/list",	type : "GET", dataSrc: "mappings", data: { "classe": "DI" },
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
         ajax: {	url : "/api/process/modbus/map/list",	type : "GET", dataSrc: "mappings", data: { "classe": "DO" },
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
         ajax: {	url : "/api/process/modbus/map/list",	type : "GET", dataSrc: "mappings", data: { "classe": "AI" },
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
         ajax: {	url : "/api/process/modbus/map/list",	type : "GET", dataSrc: "mappings", data: { "classe": "AO" },
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
