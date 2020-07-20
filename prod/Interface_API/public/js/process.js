 document.addEventListener('DOMContentLoaded', Load_process, false);

/********************************************* Reload Process *****************************************************************/
 function Process_clic_reload ( thread )
  { var xhr = new XMLHttpRequest;
    xhr.open('get', "/api/process/reload/"+thread);
    /*xhr.setRequestHeader('Content-type', 'application/json');*/
    xhr.onreadystatechange = function()
     { if (xhr.status == 200)
        { /*var Response = JSON.parse(xhr.responseText);
          console.debug(Response);*/
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send();
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_debug ( thread )
  { var xhr = new XMLHttpRequest;
    button = $('#idButtonDebug_'+thread);
    if (button.attr("debug") == "true") mode = "undebug";
    else mode = "debug";
    xhr.open('get', "/api/process/"+mode+"/"+thread);
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableProcess').DataTable().ajax.reload();
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }

     };
    xhr.send();
  }
/********************************************* Reload Process *****************************************************************/
 function Process_clic_started ( thread )
  { var xhr = new XMLHttpRequest;
    button = $('#idButtonStarted_'+thread);
    if (button.attr("started") == "true") mode = "stop";
    else mode = "start";
    xhr.open('get', "/api/process/"+mode+"/"+thread);
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { var Response = JSON.parse(xhr.responseText);
          $('#idTableProcess').DataTable().ajax.reload();
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }

     };
    xhr.send();
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
            { "data": null,
              "render": function (item)
                { if (item.started==true)
                   { return("<button id='idButtonStarted_"+item.thread+"' "+
                            "class='btn btn-success btn-block btn-sm' "+
                            "data-toggle='tooltip' title='Désactiver le process' "+
                            "onclick=Process_clic_started('"+item.thread+"') started='true'>"+
                            "Actif</button>" );
                   }
                  else
                   { return("<button id='idButtonStarted_"+item.thread+"' "+
                            "class='btn btn-outline-secondary btn-block btn-sm "+
                            "data-toggle='tooltip' title='Activer le process'"+
                            "onclick=Process_clic_started('"+item.thread+"') started='false'>"+
                            "Inactif</button>" );
                   }
                },
              "title":"Started",
            },
            { "data": null,
              "render": function (item)
                { if (item.debug==true)
                   { return("<button id='idButtonDebug_"+item.thread+"' "+
                            "class='btn btn-warning btn-block btn-sm' "+
                            "data-toggle='tooltip' title='Désactiver le debug' "+
                            "onclick=Process_clic_debug('"+item.thread+"') debug='true'>"+
                            "Oui</button>" );
                   }
                  else
                   { return("<button id='idButtonDebug_"+item.thread+"' "+
                            "class='btn btn-outline-secondary btn-block btn-sm "+
                            "data-toggle='tooltip' title='Activer le debug'"+
                            "onclick=Process_clic_debug('"+item.thread+"') debug='false'>"+
                            "Non</button>" );
                   }
                },
              "title":"Debug", "className": "hidden-xs"
            },
            { "data": "objet", "title":"Description", "className": "hidden-xs" },
            { "data": "fichier", "title":"Fichier", "className": "hidden-xs" },
            { "data": null,
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.thread.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>Voir le wiki "+item.thread.toUpperCase()+"</span></a>" );
                },
              "title":"Documentation", "className": "hidden-xs"
            },
            { "data": null,
              "render": function (item)
                { return("<button class='btn btn-outline-info btn-block btn-sm' data-toggle='tooltip' title='Recharger le process' "+
                         "onclick=Process_clic_reload('"+item.thread+"')>"+
                         "Reload</button>")
                },
              "title":"Actions", "orderable": false, "className":"text-right"
            }
          ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
