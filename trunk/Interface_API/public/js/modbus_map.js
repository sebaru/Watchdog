 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Go_to_modbus_run ()
  { Redirect ( "/tech/modbus_run" );
  }
 function Go_to_modbus_map ()
  { Redirect ( "/tech/modbus_map" );
  }/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Modbus_Del ( type, map_tech_id, map_tag )
  { var xhr = new XMLHttpRequest;
    xhr.open('DELETE', "/api/process/modbus/map/del" );
    var json_request = JSON.stringify(
       { type       : type,
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
 function Valider_Edit_DI ( id )
  { if ($('#idModalEditDI #idModalEditValider').hasClass("disabled")) return;
    $('#idModalEditDI').modal("hide");
    table = $('#idTableModbusMapDI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/process/modbus/map/set" );
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { type       : 'DI',
         tech_id    : $('#idModalEditTechID').val().toUpperCase(),
         acronyme   : $('#idModalEditAcronyme').val().toUpperCase(),
         map_tech_id: selection.map_tech_id,
         map_tag    : selection.map_tag,
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableModbusMapDI').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Modal_Edit_Input_Changed (  )
  { const Ascii_charset = RegExp(/^[a-zA-Z0-9][a-zA-Z0-9_]*$/);


    if ($('#idModalEditTechID').val().length==0)
     { $('#idModalEditDI #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#idModalEditTechIDPropose').text("Aucun match");
       $('#idModalEditDI #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
       $('#idModalEditDI #idModalEditAcronyme').prop("disabled", true);
       $('#idModalEditAcronyme').val("");
       $('#idModalEditAcronymePropose').text("Aucun match");
       return;
     }

    if (!Ascii_charset.test($('#idModalEditTechID').val()))
     { $('#idModalEditDI #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#idModalEditTechIDPropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       return;
     }

    if ($('#idModalEditAcronyme').val().length && !Ascii_charset.test($('#idModalEditAcronyme').val()))
     { $('#idModalEditDI #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-danger");
       $('#idModalEditAcronymePropose').text("Caractères autorisés : A-Z, 0-9 et _ (sauf au début)");
       return;
     }

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/mnemos/validate/" + $('#idModalEditTechID').val() + "/" + $('#idModalEditAcronyme').val() );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { var Response = JSON.parse(xhr.responseText);
          var tech_id_found=false, tech_id_propose="";
          var acronyme_found=false, acronyme_propose="";

          for (var i = 0; i < Response.nbr_tech_ids_found; i++)
           { tech_id_propose=tech_id_propose + Response.tech_ids_found[i].tech_id+" ";
             if (Response.tech_ids_found[i].tech_id == $('#idModalEditDI #idModalEditTechID').val().toUpperCase()) tech_id_found=true;
           }
          if (tech_id_found==true)
           { $('#idModalEditDI #idModalEditTechID').removeClass("bg-danger bg-warning");
             $('#idModalEditTechIDPropose').text("Match !");
             $('#idModalEditDI #idModalEditAcronyme').prop("disabled", false);
           }
          else
           { $('#idModalEditDI #idModalEditTechID').removeClass("bg-danger bg-warning").addClass("bg-warning");
             $('#idModalEditTechIDPropose').text("Choix: " + tech_id_propose);
             $('#idModalEditDI #idModalEditAcronyme').prop("disabled", true);
             $('#idModalEditAcronyme').val("");
           }

          for (var i = 0; i < Response.acronymes_found.length; i++)
           { acronyme_propose=acronyme_propose + Response.acronymes_found[i].acronyme+" ";
             if (Response.acronymes_found[i].acronyme == $('#idModalEditDI #idModalEditAcronyme').val().toUpperCase()) acronyme_found=true;
           }
          if (acronyme_found==true)
           { $('#idModalEditDI #idModalEditAcronyme').removeClass("bg-danger bg-warning");
             $('#idModalEditAcronymePropose').text("Match !");
           }
          else
           { $('#idModalEditDI #idModalEditAcronyme').removeClass("bg-danger bg-warning").addClass("bg-warning");
             $('#idModalEditAcronymePropose').text("Choix: " + acronyme_propose);
           }

          if (tech_id_found==true && acronyme_found==true)
           { $('#idModalEditDI #idModalEditValider').removeClass("disabled"); }
          else
           { $('#idModalEditDI #idModalEditValider').addClass("disabled");    }
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send();
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DI ( id )
  { table = $('#idTableModbusMapDI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
    $('#idModalEditDI #idModalEditTitre').text ( "Editer MAP - " + selection.map_tech_id + ":" + selection.map_tag );
    $('#idModalEditDI #idModalEditTechID').val ( selection.tech_id );
    $('#idModalEditDI #idModalEditAcronyme').val ( selection.acronyme );
    $('#idModalEditDI #idModalEditWagoRef').val  ( selection.map_tech_id + ":" + selection.map_tag );
    $('#idModalEditDI #idModalEditWagoRef').attr ( "readonly", true );
    $('#idModalEditDI #idModalEditValider').attr( "onclick", "Valider_Edit_DI('"+id+"')" );
    $('#idModalEditDI').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_DO ( map )
  { table = $('#idTableModbusMapDO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.map_tag==map) } )[0];
    Show_Modal_Map_Del ( "DO", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AI ( map )
  { table = $('#idTableModbusMapAI').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.map_tag==map) } )[0];
    Show_Modal_Map_Del ( "AI", selection )
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Map_Edit_AO ( map )
  { table = $('#idTableModbusMapAO').DataTable();
    selection = table.ajax.json().mappings.filter( function(item) { return (item.map_tag==map) } )[0];
    Show_Modal_Map_Del ( "AO", selection )
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
  { $('#idTableModbusMapDI').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/process/modbus/map/DI",	type : "GET", dataSrc: "mappings",
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le mapping", "Show_Modal_Map_Edit_DI", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le mapping", "Show_Modal_Map_Del_DI", item.id, "trash", null );
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
         ajax: {	url : "/api/process/modbus/map/DO",	type : "GET", dataSrc: "mappings",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Wago Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[0] ) }
            },
            { "data": null, "title":"Wago I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[1] ) }
            },
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "Show_Modal_Map_Edit_DO", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "Show_Modal_Map_Del_DO", item.id, "trash", null );
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
         ajax: {	url : "/api/process/modbus/map/AI",	type : "GET", dataSrc: "mappings",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Wago Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[0] ) }
            },
            { "data": null, "title":"Wago I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[1] ) }
            },
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "Show_Modal_Map_Edit_AI", item.map_tag, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "Show_Modal_Map_Del_AI", item.map_tag, "trash", null );
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
         ajax: {	url : "/api/process/modbus/map/AO",	type : "GET", dataSrc: "mappings",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Wago Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[0] ) }
            },
            { "data": null, "title":"Wago I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.map_tag.split(':')[1] ) }
            },
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
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "Show_Modal_Map_Edit_AO", item.map_tag, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "Show_Modal_Map_Del_AI", item.map_tag, "trash", null );
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
