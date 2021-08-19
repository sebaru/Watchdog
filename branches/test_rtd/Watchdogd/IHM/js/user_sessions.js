/********************************************* Reload Process *****************************************************************/
 function Users_sessions_clic_kill ( session )
  { var json_request = JSON.stringify( { id: session } );
    Send_to_API ( "DELETE", "/api/users/kill", json_request, function ()
     { $('#idTableUserSessions').DataTable().ajax.reload( null ,false );
     }, null );
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_mes_sessions ( )
  { var bodymain = $("#bodymain");
    bodymain.append ( $("<h4></h4>" ).text( "Mes appareils" ).addClass("text-white align-items-center") )
            .append ( $("<table></table>")
                      .attr("id","idTableUserSessions")
                      .addClass("table table-dark table-bordered"));
    $('#idTableUserSessions').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/users/sessions",	type : "GET", dataSrc: "Sessions",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "username", "title":"Nom", "className": "bg-dark text-center" },
            { "data": "appareil", "title":"Appareil", "className": "bg-dark text-center" },
            { "data": "useragent", "title":"Navigateur", "className": "bg-dark d-none d-sm-table-cell text-center" },
            { "data": "host", "title":"Host", "className": "bg-dark d-none d-sm-table-cell text-center" },
            { "data": null, "title":"Level", "className": "bg-dark text-center",
              "render": function (item)
                { return ( Badge_Access_level ( item.access_level ) ); }
            },
            { "data": null, "title":"Dernier accès", "className": "bg-dark d-none d-sm-table-cell",
              "render": function (item)
                { dateObj = new Date(item.last_request*1000);
                  return(dateObj.toLocaleString());
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "className": "bg-dark",
              "render": function (item)
                { return("<button class='btn btn-danger btn-block btn-sm' data-toggle='tooltip' title='Supprimer la session' "+
                         "onclick=Users_sessions_clic_kill('"+item.id+"')>"+
                         "<i class='fas fa-skull'></i> Kill</button>")
                },
            }
          ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
    Slide_down_when_loaded ( "toplevel" );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Charger_page_user_sessions ( )
  { console.log("Charger_page_user_session ");
    Scroll_to_top();
    $('#toplevel').slideUp("normal", function ()
     { $('#toplevel').empty()
                     .append("<div id='bodymain'></div>").addClass('mx-1');
       Charger_mes_sessions ();
     });
  }
