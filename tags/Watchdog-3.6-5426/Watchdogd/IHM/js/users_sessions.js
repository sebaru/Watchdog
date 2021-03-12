 document.addEventListener('DOMContentLoaded', Load_users_sessions, false);

/********************************************* Reload Process *****************************************************************/
 function Users_sessions_clic_kill ( session )
  { var xhr = new XMLHttpRequest;
    xhr.open('DELETE', "/api/users/kill");
    xhr.setRequestHeader('Content-type', 'application/json');
    var json_request = JSON.stringify( { wtd_session: session } );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { /*var Response = JSON.parse(xhr.responseText);*/
/*          button = $('#idButtonStarted_'+Response.thread).attr("started", Response.started );
          if (Response.started==false)
           { button.removeClass("btn-success").addClass("btn-outline-secondary").html("Inactif").attr("title","Activer le process"); }
          else
           { button.removeClass("btn-outline-secondary").addClass("btn-success").html("Actif").attr("title","Désactiver le process"); }*/
          $('#idTableUsersSessions').DataTable().ajax.reload();
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send(json_request);
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_users_sessions ()
  { $('#idTableUsersSessions').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/users/sessions",	type : "GET", dataSrc: "Sessions",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "username", "title":"Username", "className": "text-center" },
            { "data": "host", "title":"Host", "className": "text-center" },
            { "data": "access_level", "title":"Level", "className": "" },
            { "data": "wtd_session", "title":"Wtd_session", "className": "" },
            { "data": null, "title":"Dernier accès", "className": "",
              "render": function (item)
                { dateObj = new Date(item.last_request*1000);
                  return(dateObj.toLocaleString());
                }
            },
            { "data": null, "title":"Actions", "orderable": false,
              "render": function (item)
                { return("<button class='btn btn-danger btn-block btn-sm' data-toggle='tooltip' title='Supprime la session' "+
                         "onclick=Users_sessions_clic_kill('"+item.wtd_session+"')>"+
                         "<i class='fas fa-skull'></i> Kill</button>")
                },
            }
          ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
