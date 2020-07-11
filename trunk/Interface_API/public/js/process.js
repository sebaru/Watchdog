 document.addEventListener('DOMContentLoaded', Load_process, false);

 function Process_reload ( tech_id )
  { var xhr = new XMLHttpRequest;
    xhr.open('get', "/api/process/reload/"+tech_id);
    /*xhr.setRequestHeader('Content-type', 'application/json');*/
    xhr.onreadystatechange = function()
     { /*$('#idGlobalStatus').fadeIn('fast')
       setTimeout ( function () { $('#idGlobalStatus').fadeOut(); }, 1000 );*/

       if (xhr.status == 200)
        { /*var Response = JSON.parse(xhr.responseText);
          console.debug(Response);*/
          $('#idToastStatus').toast('show');
          /*$('#idToastAlert').toast("show");*/
        }
       else { $('#idErrorDetail').innerHtml = "Une erreur s'est produite...";
              $('#idModalError').modal("show");
            }
     };
    xhr.send();
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_process ()
  { console.log ("in load process !");
    $('#idTableProcess').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/process/list",	type : "GET", dataSrc: "Process" },
         columns:
          [ { "data": "tech_id", "title":"Tech_ID", "className": "text-center" },
            { "data": null,
              "render": function (item)
                { if (item.started==true)
                   { return("<button class='btn btn-success btn-block btn-sm' data-toggle='tooltip' title='Désactiver le process'>Actif</button>" ); }
                  else
                   { return("<button class='btn btn-outline-secondary btn-block btn-sm data-toggle='tooltip' title='Activer le process'>Inactif</button>" ); }
                },
              "title":"Started",
            },
            { "data": null,
              "render": function (item)
                { if (item.debug==true)
                   { return("<button class='btn btn-warning btn-block btn-sm data-toggle='tooltip' title='Désactiver le debug'>Oui</button>" ); }
                  else
                   { return("<button class='btn btn-outline-secondary btn-block btn-sm data-toggle='tooltip' title='Activer le debug'>Non</button>" ); }
                },
              "title":"Debug", "className": "hidden-xs"
            },
            { "data": "objet", "title":"Description" },
            { "data": "fichier", "title":"Fichier" },
            { "data": null,
              "render": function (item)
                { return("<a href='https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_"+item.tech_id.toUpperCase()+"_Thread'>"+
                         "<span class='label label-info'>Voir le wiki "+item.tech_id.toUpperCase()+"</span></a>" );
                },
              "title":"Documentation", "className": "hidden-xs"
            },
            { "data": null,
              "render": function (item)
                { return("<button class='btn btn-info btn-block btn-sm' data-toggle='tooltip' title='Recharger le process' "+
                         "onclick=Process_reload('"+item.tech_id+"')>"+
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
