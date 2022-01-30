 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Reload Process *****************************************************************/
 function Process_clic_debug ( uuid )
  { var json_request = JSON.stringify(
       { uuid : uuid,
         debug: true,
       }
     );
    Send_to_API ( "POST", "/api/process/debug", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_undebug ( uuid )
  { var json_request = JSON.stringify(
       { uuid : uuid,
         debug: false,
       }
     );
    Send_to_API ( "POST", "/api/process/debug", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_enable ( uuid )
  { var json_request = JSON.stringify(
       { uuid  : uuid,
         status: true,
       }
     );
    $("#idButtonSpinner_"+uuid).show();
    Send_to_API ( "POST", "/api/process/enable", json_request, function(Response)
     { setTimeout ( function () { $('#idTableProcess').DataTable().ajax.reload(); }, 2000 );
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_disable ( uuid )
  { var json_request = JSON.stringify(
       { uuid  : uuid,
         status: false,
       }
     );
    $("#idButtonSpinner_"+uuid).show();
    Send_to_API ( "POST", "/api/process/enable", json_request, function(Response)
     { setTimeout ( function () { $('#idTableProcess').DataTable().ajax.reload(); }, 2000 );
     }, null);
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_page ()
  { $('#idTableProcess').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/process/list",	type : "GET", dataSrc: "Process",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "instance", "title":"Instance", "className": "align-middle text-center" },
            { "data": null, "title":"Name", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/"+item.name, "Voir la conf du process "+item.name, item.name ) ); }
            },
            { "data": null, "title":"UUID", "className": "align-middle text-center",
              "render": function (item)
                { return( item.uuid.slice(0, 8) ); }
            },
            { "data": null, "title":"Enabled", "className": "align-middle text-center",
              "render": function (item)
                { if (item.enable==true)
                   { return( Bouton ( "success", "Désactiver le process",
                                      "Process_clic_disable", item.uuid, "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le process",
                                      "Process_clic_enable", item.uuid, "Non" ) );
                   }
                },
            },
            { "data": null, "title":"Debug", "className": "align-middle text-center",
              "render": function (item)
                { if (item.debug==true)
                   { return( Bouton ( "warning", "Désactiver le debug",
                                      "Process_clic_undebug", item.uuid, "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le debug",
                                      "Process_clic_debug", item.uuid, "Non" ) );
                   }
                }
            },
            { "data": null, "title":"Version", "className": "align-middle text-center",
              "render": function (item)
                { if(item.version == null) return("--");
                  return(item.version);
                },
            },
            { "data": "start_time", "title":"Start Time", "className": "align-middle text-center" },
/*            { "data": null, "title":"Start Time", "className": "align-middle text-center",
              "render": function (item)
                { if(item.start_time == null) return("--");
                  var myDate = new Date( item.start_time *1000 );
                  return(myDate.toLocaleString());
                },
            },*/
            { "data": null, "title":"Description", "className": "align-middle",
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.name.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>"+item.description+"</span></a>" );
                },

            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-right",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-danger", "Decharger/Recharger le process",
                                                  "Process_reload", item.uuid, "redo", "Reload" );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         //order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
