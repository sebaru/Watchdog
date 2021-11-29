 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Refresh ( )
  { $('#idTableMsrv').DataTable().ajax.reload(null, false); }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Sauver_parametre ( id )
  { table = $('#idTableMsrv').DataTable();
    selection = table.ajax.json().instances.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { host       : selection.host,
       description: $("#idMSRVDescription_"+id).val(),
       log_level  : parseInt($("#idMSRVLogLevel_"+id).val()),
       debug      : ($("#idMSRVDebug_"+id).val()=="true" ? true : false),
       log_db     : ($("#idMSRVLogDB_"+id).val()=="true" ? true : false),
       log_zmq    : ($("#idMSRVLogZMQ_"+id).val()=="true" ? true : false),
       log_trad   : ($("#idMSRVLogTRAD_"+id).val()=="true" ? true : false),
     };
    Send_to_API ( 'POST', "/api/instance/set", JSON.stringify(json_request), function ()
     { $('#idTableMsrv').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Reset_Valider ( selection )
  { var json_request = { host: selection.host };
    Send_to_API ( 'POST', "/api/instance/reset", JSON.stringify(json_request), function ()
     { $('#idTableMsrv').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Reset ( id  )
  { table = $('#idTableMsrv').DataTable();
    selection = table.ajax.json().instances.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Restarter l'instance "+selection.host,
                     "Etes-vous sûr de vouloir relancer cette instance ?",
                     selection.host + " - "+selection.description,
                     function () { MSRV_Reset_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function MSRV_Reload_icons ( )
  { Send_to_API ( 'POST', "/api/instance/reload_icons", null, null, null ); }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableMsrv').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/instance/list", type : "GET", dataSrc: "instances",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "host",   "title":"Host",   "className": "align-middle text-center" },
           { "data": null, "title":"Master", "className": "align-middle text-center",
             "render": function (item)
              { if (item.is_master==true)
                 { return( Bouton ( "success", "Instance is Master", null, null, "Yes" ) ); }
                else
                 { return( Bouton ( "secondary", "Instance is Slave", null, null, "No" ) ); }
              }
           },
           { "data": "version", "title":"Version",   "className": "align-middle text-center" },
           { "data": "database_version", "title":"Database",   "className": "align-middle text-center" },
           { "data": null, "title":"Description", "className": "align-middle ",
             "render": function (item)
              { return( Input ( "text", "idMSRVDescription_"+item.id, "MSRV_Sauver_parametre("+item.id+")",
                                "Description de l'instance", item.description, null ) );
              }
           },
           { "data": null, "title":"Debug", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idMSRVDebug_"+item.id, "MSRV_Sauver_parametre("+item.id+")", choix, item.debug ) );
              }
           },
           { "data": null, "title":"Log_DB", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idMSRVLogDB_"+item.id, "MSRV_Sauver_parametre("+item.id+")", choix, item.log_db ) );
              }
           },
           { "data": null, "title":"Log_ZMQ", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idMSRVLogZMQ_"+item.id, "MSRV_Sauver_parametre("+item.id+")", choix, item.log_zmq ) );
              }           },
           { "data": null, "title":"Log_Trad", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idMSRVLogTRAD_"+item.id, "MSRV_Sauver_parametre("+item.id+")", choix, item.log_trad ) );
              }
           },
           { "data": null, "title":"Log Level", "className": "align-middle ",
             "render": function (item)
              { var choix = [ { valeur: 7, texte: "Debug" },
                              { valeur: 6, texte: "Info" },
                              { valeur: 5, texte: "Notice" },
                              { valeur: 4, texte: "Warning" },
                              { valeur: 3, texte: "Error" } ];
                return( Select ( "idMSRVLogLevel_"+item.id, "MSRV_Sauver_parametre("+item.id+")", choix, item.log_level ) );
              }
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "danger", "Restart l'instance", "MSRV_Reset", item.id, "redo", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
  }
