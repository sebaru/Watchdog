 document.addEventListener('DOMContentLoaded', Load_process, false);

/********************************************* Reload Process *****************************************************************/
 function Process_clic_reload ( thread, hard )
  { var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/reload");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { thread : thread,
         hard   : hard,
       }
     );
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { /*var Response = JSON.parse(xhr.responseText);
          console.debug(Response);*/
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_debug ( thread, enable )
  { var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/debug");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { thread : thread,
         status : enable,
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableProcess').DataTable().ajax.reload();
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }

     };
    xhr.send(json_request);
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_started ( thread, enable )
  { var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/start");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { thread : thread,
         status : enable,
       }
     );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { //var Response = JSON.parse(xhr.responseText);
          $('#idTableProcess').DataTable().ajax.reload();
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }

     };
    xhr.send(json_request);
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_process ()
  { console.log ("in load process !");

    $('#idTableProcess').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/process/list",	type : "GET", dataSrc: "Process",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "thread", "title":"Thread", "className": "text-center" },
            { "data": null, "title":"Started",
              "render": function (item)
                { if (item.started==true)
                   { return( Bouton ( "success", "Désactiver le process",
                                      "Process_clic_start", item.thread+",false", "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le process",
                                      "Process_clic_start", item.thread+",true", "Inactif" ) );
                   }
                },
            },
            { "data": null, "title":"Debug", "className": "hidden-xs",
              "render": function (item)
                { if (item.debug==true)
                   { return( Bouton ( "warning", "Désactiver le debug",
                                      "Process_clic_debug", item.thread+",false", "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le debug",
                                      "Process_clic_debug", item.thread+",true", "Non" ) );
                   }
                }
            },
            { "data": "version", "title":"Version", "className": "text-center hidden-xs" },
            { "data": "objet", "title":"Description", "className": "hidden-xs" },
            { "data": "fichier", "title":"Fichier", "className": "hidden-xs" },
            { "data": null, "title":"Documentation", "className": "hidden-xs",
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.thread.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>Voir le wiki "+item.thread.toUpperCase()+"</span></a>" );
                },
              
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"text-right",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-info", "Recharger le thread", "Process_clic_reload", item.thread+",false", "redo", "Soft Reload" );
                  boutons += Bouton_actions_add ( "outline-danger", "Decharger/Recharger le thread", "Process_clic_reload", item.thread+",true", "redo", "Hard Reload" );
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
