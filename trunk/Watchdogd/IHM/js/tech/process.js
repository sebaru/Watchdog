 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Reload Process *****************************************************************/
 function Process_clic_reload ( params )
  { parametres = params.split(':');
    var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/reload");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { instance: parametres[0],
         thread  : parametres[1],
         hard    : (parametres[2] === "true" ? true : false),
       }
     );
    xhr.onreadystatechange = function()
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
 function Process_clic_debug ( params )
  { parametres = params.split(':');
    var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/debug");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { instance: parametres[0],
         thread  : parametres[1],
         status  : (parametres[2] === "true" ? true : false),
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
 function Process_clic_start ( params )
  { parametres = params.split(':');
    var xhr = new XMLHttpRequest;
    xhr.open('PUT', "/api/process/start");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify(
       { instance: parametres[0],
         thread  : parametres[1],
         status : (parametres[2] === "true" ? true : false),
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
 function Load_page ()
  { console.log ("in load process !");

    $('#idTitleProcessus').text(Get_locale_instance());

    $('#idTableProcess').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/process/list",	type : "GET", dataSrc: "Process",
                 data: { "instance": Get_locale_instance() },
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"Thread", "className": "text-center",
              "render": function (item)
                { return( Lien ( "/tech/"+item.thread, "Voir la conf du thread "+item.thread, item.thread ) ); }
            },
            { "data": null, "title":"Started",
              "render": function (item)
                { if (item.started==true)
                   { return( Bouton ( "success", "Désactiver le process",
                                      "Process_clic_start", instance+":"+item.thread+":false", "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le process",
                                      "Process_clic_start", instance+":"+item.thread+":true", "Inactif" ) );
                   }
                },
            },
            { "data": null, "title":"Debug", "className": "hidden-xs",
              "render": function (item)
                { if (item.debug==true)
                   { return( Bouton ( "warning", "Désactiver le debug",
                                      "Process_clic_debug", instance+":"+item.thread+":false", "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le debug",
                                      "Process_clic_debug", instance+":"+item.thread+":true", "Non" ) );
                   }
                }
            },
            { "data": "version", "title":"Version", "className": "text-center hidden-xs" },
            { "data": null, "title":"Start Time", "className": "text-center hidden-xs",
              "render": function (item)
                { if(item.start_time == 0) return("--");
                  var myDate = new Date( item.start_time *1000 );
                  return(myDate.toLocaleString());
                },

            },
            { "data": "objet", "title":"Description", "className": "hidden-xs" },
            { "data": null, "title":"Documentation", "className": "hidden-xs",
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.thread.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>Voir le wiki "+item.thread.toUpperCase()+"</span></a>" );
                },

            },
            { "data": null, "title":"Actions", "orderable": false, "className":"text-right",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-info", "Recharger le thread", "Process_clic_reload", instance+":"+item.thread+":false", "redo", "Soft Reload" );
                  boutons += Bouton_actions_add ( "outline-danger", "Decharger/Recharger le thread", "Process_clic_reload", instance+":"+item.thread+":true", "redo", "Hard Reload" );
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
