 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Refresh ( )
  { $('#idTableINSTANCE').DataTable().ajax.reload(null, false); }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Sauver_parametre ( id )
  { table = $('#idTableINSTANCE').DataTable();
    selection = table.ajax.json().instances.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { instance   : selection.instance,
       description: $("#idINSTANCEDescription_"+id).val(),
       log_level  : parseInt($("#idINSTANCELogLevel_"+id).val()),
       log_msrv   : ($("#idINSTANCELogMSRV_"+id).val()=="true" ? true : false),
       log_db     : ($("#idINSTANCELogDB_"+id).val()=="true" ? true : false),
       log_zmq    : ($("#idINSTANCELogZMQ_"+id).val()=="true" ? true : false),
       log_trad   : ($("#idINSTANCELogTRAD_"+id).val()=="true" ? true : false),
     };
    Send_to_API ( 'POST', "/api/instance/set", JSON.stringify(json_request), function ()
     { INSTANCE_Refresh ();
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Reset_Valider ( selection )
  { var json_request = { zmq_tag: "INSTANCE_RESET", thread_tech_id: selection.instance };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), function ()
     { Show_Info ( "Attendez le redémarrage" );
       Reload_when_ready();
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Reset ( id  )
  { table = $('#idTableINSTANCE').DataTable();
    selection = table.ajax.json().instances.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Restarter cette instance "+selection.instance,
                     "Etes-vous sûr de vouloir relancer cette instance ?",
                     selection.instance + " - "+selection.description,
                     function () { INSTANCE_Reset_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Upgrade_Valider ( selection )
  { var json_request = { zmq_tag: "INSTANCE_UPGRADE", thread_tech_id: selection.instance };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), function ()
     { Show_Info ( "Attendez le download et le redémarrage" );
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Upgrade ( id  )
  { table = $('#idTableINSTANCE').DataTable();
    selection = table.ajax.json().instances.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Upgrader cette instance "+selection.instance,
                     "Etes-vous sûr de vouloir upgrader cette instance ?",
                     selection.instance + " - "+selection.description,
                     function () { INSTANCE_Upgrade_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function INSTANCE_Reload_icons ( )
  { Send_to_API ( 'POST', "/api/instance/reload_icons", null, null, null ); }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableINSTANCE').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/instance/list", type : "GET", dataSrc: "instances",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": null, "title":"Master", "className": "align-middle text-center",
             "render": function (item)
              { if (item.is_master==true)
                 { return( Bouton ( "success", "Instance is Master", null, null, "Yes" ) ); }
                else
                 { return( Bouton ( "secondary", "Master is "+item.master_host, null, null, item.master_host ) ); }
              }
           },
           { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": "version", "title":"Version",   "className": "align-middle text-center" },
           { "data": "database_version", "title":"Database",   "className": "align-middle text-center" },
           { "data": "start_time", "title":"Start time",   "className": "align-middle text-center" },
           { "data": null, "title":"Description", "className": "align-middle ",
             "render": function (item)
              { return( Input ( "text", "idINSTANCEDescription_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")",
                                "Description de l'instance", item.description, null ) );
              }
           },
           { "data": null, "title":"Log_MSRV", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idINSTANCELogMSRV_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")", choix, item.log_msrv ) );
              }
           },
           { "data": null, "title":"Log_DB", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idINSTANCELogDB_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")", choix, item.log_db ) );
              }
           },
           { "data": null, "title":"Log_ZMQ", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idINSTANCELogZMQ_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")", choix, item.log_zmq ) );
              }           },
           { "data": null, "title":"Log_Trad", "className": "align-middle text-center",
             "render": function (item)
              { var choix = [ { valeur: false, texte: "No" }, { valeur: true, texte: "Yes" } ];
                return( Select ( "idINSTANCELogTRAD_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")", choix, item.log_trad ) );
              }
           },
           { "data": null, "title":"Log Level", "className": "align-middle ",
             "render": function (item)
              { var choix = [ { valeur: 7, texte: "Debug" },
                              { valeur: 6, texte: "Info" },
                              { valeur: 5, texte: "Notice" },
                              { valeur: 4, texte: "Warning" },
                              { valeur: 3, texte: "Error" } ];
                return( Select ( "idINSTANCELogLevel_"+item.id, "INSTANCE_Sauver_parametre("+item.id+")", choix, item.log_level ) );
              }
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "warning", "Upgrade l'instance", "INSTANCE_Upgrade", item.id, "download", null );
                 boutons += Bouton_actions_add ( "danger", "Restart l'instance", "INSTANCE_Reset", item.id, "redo", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       order: [ [0, "desc"] ],
       responsive: true,
     });
  }
