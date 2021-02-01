 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Reload Process *****************************************************************/
 function Process_clic_soft_reload ( thread )
  { Process_reload ( $('#idTargetInstance').val(), thread, false ); }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_hard_reload ( thread )
  { Process_reload ( $('#idTargetInstance').val(), thread, true ); }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_debug ( thread )
  { instance = $('#idTargetInstance').val();
    var json_request = JSON.stringify(
       { instance: $('#idTargetInstance').val(),
         thread  : thread,
         status  : true,
       }
     );
    Send_to_API ( "POST", "/api/process/debug", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_undebug ( thread )
  { instance = $('#idTargetInstance').val();
    var json_request = JSON.stringify(
       { instance: $('#idTargetInstance').val(),
         thread  : thread,
         status  : false,
       }
     );
    Send_to_API ( "POST", "/api/process/debug", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_start ( thread )
  { instance = $('#idTargetInstance').val();
    var json_request = JSON.stringify(
       { instance: $('#idTargetInstance').val(),
         thread  : thread,
         status  : true,
       }
     );
    Send_to_API ( "POST", "/api/process/start", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_stop ( thread )
  { instance = $('#idTargetInstance').val();
    var json_request = JSON.stringify(
       { instance: $('#idTargetInstance').val(),
         thread  : thread,
         status  : false,
       }
     );
    Send_to_API ( "POST", "/api/process/start", json_request, function(Response)
     { $('#idTableProcess').DataTable().ajax.reload();
     }, null);
  }
/************************ Appelé quand l'utilisateur change la target instance dans le select *********************************/
 function Process_change_target_instance()
  { $('#idTableProcess').DataTable().ajax.url("/api/process/list?instance="+$('#idTargetInstance').val());
    $('#idTableProcess').DataTable().ajax.reload();
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_page ()
  { $('#idTargetInstance').empty();
    if (localStorage.getItem("instance_is_master")=="true")
     { Send_to_API ( "GET", "/api/instance/list", null, function (Response)
        { $('#idTargetInstance').append("<option value='MASTER'>MASTER</option>");

          $.each ( Response.instances, function ( i, instance )
           { $('#idTargetInstance').append("<option value='"+instance.instance_id+"'>"+instance.instance_id+"</option>"); } );
          $('#idTargetInstance').val("MASTER");
        }, null);
     }
    else
     { $('#idTargetInstance').append("<option value='LOCAL'>LOCAL</option>"); }

    $('#idTableProcess').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/process/list",	type : "GET", dataSrc: "Process",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Thread", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/"+item.thread, "Voir la conf du thread "+item.thread, item.thread ) ); }
            },
            { "data": null, "title":"Started", "className": "align-middle text-center",
              "render": function (item)
                { if (item.started==true)
                   { return( Bouton ( "success", "Désactiver le process",
                                      "Process_clic_stop", item.thread, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le process",
                                      "Process_clic_start", item.thread, "Inactif" ) );
                   }
                },
            },
            { "data": null, "title":"Debug", "className": "align-middle text-center",
              "render": function (item)
                { if (item.debug==true)
                   { return( Bouton ( "warning", "Désactiver le debug",
                                      "Process_clic_undebug", item.thread, "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le debug",
                                      "Process_clic_debug", item.thread, "Non" ) );
                   }
                }
            },
            { "data": "version", "title":"Version", "className": "align-middle text-center", },
            { "data": null, "title":"Start Time", "className": "align-middle text-center",
              "render": function (item)
                { if(item.start_time == 0) return("--");
                  var myDate = new Date( item.start_time *1000 );
                  return(myDate.toLocaleString());
                },

            },
            { "data": "objet", "title":"Description", "className": "align-middle" },
            { "data": null, "title":"Documentation", "className": "align-middle",
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.thread.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>Voir le wiki "+item.thread.toUpperCase()+"</span></a>" );
                },

            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-right",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-info", "Recharger le thread", "Process_clic_soft_reload", item.thread, "redo", "Soft Reload" );
                  boutons += Bouton_actions_add ( "outline-danger", "Decharger/Recharger le thread", "Process_clic_hard_reload", item.thread, "redo", "Hard Reload" );
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
