 document.addEventListener('DOMContentLoaded', Load_page, false);

 function MODBUS_Refresh ( )
  { $('#idTableMODBUS').DataTable().ajax.reload(null, false);
    $('#idTableMODBUS_DI').DataTable().ajax.reload(null, false);
    $('#idTableMODBUS_DO').DataTable().ajax.reload(null, false);
    $('#idTableMODBUS_AI').DataTable().ajax.reload(null, false);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Disable ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable        : false,
       uuid          : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id            : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       MODBUS_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Enable ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable        : true,
       uuid          : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id            : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       MODBUS_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function MODBUS_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       MODBUS_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function MODBUS_Del ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.hostname +" - "+ selection.description,
                     function () { MODBUS_Del_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MODBUS_Set ( selection )
  { var json_request =
     { uuid:           $('#idTargetProcess').val(),
       thread_tech_id: $('#idMODBUSTechID').val(),
       hostname:       $('#idMODBUSHostname').val(),
       description:    $('#idMODBUSDescription').val(),
       watchdog:       parseInt($('#idMODBUSWatchdog').val()),
       max_request_par_sec: parseInt($('#idMODBUSMaxRequestParSec').val()),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       MODBUS_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit ( id )
  { table = $('#idTableMODBUS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=modbus", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idMODBUSTitre').text("Editer la connexion MODBUS " + selection.thread_tech_id);
    $('#idMODBUSTechID').val( selection.thread_tech_id )
      .off("input").on("input", function () { Controle_tech_id( "idMODBUS", selection.thread_tech_id ); } ).trigger("input");
    $('#idMODBUSHostname').val ( selection.hostname );
    $('#idMODBUSDescription').val( selection.description );
    $('#idMODBUSWatchdog').val( selection.watchdog );
    $('#idMODBUSMaxRequestParSec').val( selection.max_request_par_sec );
    $('#idMODBUSValider').off("click").on( "click", function () { MODBUS_Set(selection); } );
    $('#idMODBUSEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Add ( )
  { $('#idMODBUSTitre').text("Ajouter un MODBUS");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=modbus", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idMODBUSTechID').val("").off("input").on("input", function () { Controle_tech_id( "idMODBUS", null ); } );
    $('#idMODBUSHostname').val ( "" );
    $('#idMODBUSDescription').val( "" );
    $('#idMODBUSWatchdog').val( "" );
    $('#idMODBUSMaxRequestParSec').val( "" );
    $('#idMODBUSValider').off("click").on( "click", function () { MODBUS_Set(null); } );
    $('#idMODBUSEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Map_DI ( id )
  { table = $('#idTableMODBUS_DI').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMODALMapTitre').text( "Mapper "+selection.thread_tech_id+":"+selection.thread_acronyme );
    $('#idMODALMapRechercherTechID').off("input").on("input", function () { Common_Updater_Choix_TechID ( "idMODALMap", "DI" ); } );
    Common_Updater_Choix_TechID ( "idMODALMap", "DI", selection.tech_id, selection.acronyme );
    $('#idMODALMapValider').off("click").on( "click", function ()
     { $('#idMODALMap').modal("hide");
       COMMON_Map ( selection.thread_tech_id, selection.thread_acronyme,
                    $('#idMODALMapSelectTechID').val(),  $('#idMODALMapSelectAcronyme').val()
                  );
       MODBUS_Refresh();;
     });
    $('#idMODALMap').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Map_DO ( id )
  { table = $('#idTableMODBUS_DO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMODALMapTitre').text( "Mapper "+selection.thread_tech_id+":"+selection.thread_acronyme );
    $('#idMODALMapRechercherTechID').off("input").on("input", function () { Common_Updater_Choix_TechID ( "idMODALMap", "DO" ); } );
    Common_Updater_Choix_TechID ( "idMODALMap", "DO", selection.tech_id, selection.acronyme );
    $('#idMODALMapValider').off("click").on( "click", function ()
     { $('#idMODALMap').modal("hide");
       COMMON_Map ( selection.thread_tech_id, selection.thread_acronyme,
                    $('#idMODALMapSelectTechID').val(),  $('#idMODALMapSelectAcronyme').val()
                  );
       MODBUS_Refresh();
     });
    $('#idMODALMap').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Map_AI ( id )
  { table = $('#idTableMODBUS_AI').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMODALMapTitre').text( "Mapper "+selection.thread_tech_id+":"+selection.thread_acronyme );
    $('#idMODALMapRechercherTechID').off("input").on("input", function () { Common_Updater_Choix_TechID ( "idMODALMap", "AI" ); } );
    Common_Updater_Choix_TechID ( "idMODALMap", "AI", selection.tech_id, selection.acronyme );
    $('#idMODALMapValider').off("click").on( "click", function ()
     { $('#idMODALMap').modal("hide");
       COMMON_Map ( selection.thread_tech_id, selection.thread_acronyme,
                    $('#idMODALMapSelectTechID').val(),  $('#idMODALMapSelectAcronyme').val()
                  );
       MODBUS_Refresh();
     });
    $('#idMODALMap').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function MODBUS_Edit_AI ( id )
  { table = $('#idTableMODBUS_AI').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idMODBUSEditAI').text( "Configurer "+selection.thread_tech_id+":"+selection.thread_acronyme );
    $('#idMODBUSEditAITypeBorne').val ( selection.type_borne );
    $('#idMODBUSEditAIMin').val ( selection.min );
    $('#idMODBUSEditAIMax').val ( selection.max );
    $('#idMODBUSEditAIValider').off("click").on( "click", function ()
     { $('#idMODBUSEditAI').modal("hide");
       /*Send_to_API().then ( () => { MODBUS_Refresh(); } );*/
     });
    $('#idMODBUSEditAI').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMODBUS').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/config", type : "GET", data: { name: "modbus" }, dataSrc: "config",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
            { "data": null, "title":"Enable", "className": "align-middle text-center",
               "render": function (item)
                { if (item.enable==true)
                  { return( Bouton ( "success", "Désactiver le module", "MODBUS_Disable", item.id, "Actif" ) ); }
                 else
                  { return( Bouton ( "outline-secondary", "Activer le module", "MODBUS_Enable", item.id, "Désactivé" ) ); }
                },
            },
            { "data": null, "title":"Tech_id", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
            },
            { "data": "description", "title":"Description", "className": "align-middle text-center " },
            { "data": "watchdog", "title":"Watchdog (s)", "className": "align-middle text-center " },
            { "data": "hostname", "title":"Hostname", "className": "align-middle text-center " },
            { "data": "max_request_par_sec", "title":"Max Requete/s", "className": "align-middle text-center " },
            { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
              "render": function (item)
                { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                  else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
                },
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer le module", "MODBUS_Edit", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le module", "MODBUS_Del", item.id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableMODBUS_DI').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id", paging: false,
         ajax: {	url : "/api/process/config", type : "GET", data: { name: "modbus", classe: "DI" }, dataSrc: "config",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"WAGO TechID", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
            },
            { "data": null, "title":"WAGO I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.thread_acronyme ); }
            },
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
                  boutons += Bouton_actions_add ( "primary", "Mapper cet objet", "MODBUS_Map_DI", item.id, "directions", null );
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
         ajax: {	url : "/api/process/config", type : "GET", data: { name: "modbus", classe: "DO" }, dataSrc: "config",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"WAGO TechID", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
            },
            { "data": null, "title":"WAGO I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.thread_acronyme ); }
            },
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
                  boutons += Bouton_actions_add ( "primary", "Mapper cet objet", "MODBUS_Map_DO", item.id, "directions", null );
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
         ajax: {	url : "/api/process/config", type : "GET", data: { name: "modbus", classe: "AI" }, dataSrc: "config",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"WAGO TechID", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
            },
            { "data": null, "title":"WAGO I/O", "className": "align-middle text-center",
              "render": function (item)
                { return( item.thread_acronyme ); }
            },
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
            { "data": null, "title":"Type Borne", "className": "align-middle text-center",
              "render": function (item)
                { return( "type_borne = " + item.type_borne ); }
            },
            { "data": "min", "title":"min", "className": "align-middle text-center" },
            { "data": "max", "title":"max", "className": "align-middle text-center" },
            { "data": null, "title":"Unité", "className": "align-middle text-center",
              "render": function (item)
                { return( htmlEncode ( item.unite ) ); }
            },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "primary", "Editer cet objet", "MODBUS_Edit_AI", item.id, "pen", null );
                  boutons += Bouton_actions_add ( "primary", "Mapper cet objet", "MODBUS_Map_AI", item.id, "directions", null );
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

         responsive: true,
       }
     );*/

    $('#idTabEntreeTor').tab('show');
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
