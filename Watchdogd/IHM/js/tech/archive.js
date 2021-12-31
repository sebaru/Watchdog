 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Archive_purge ( )
  { Send_to_API ( 'PUT', "/api/archive/purge", null, function (Response)
     { $('#idModalInfoDetail').html("Execution du thread de purge: <strong>"+Response.exec_purge_thread+"</strong>" );
       $('#idModalInfo').modal("show");
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Archive_clear (  )
  { Send_to_API ( 'PUT', "/api/archive/clear", null, function (Response)
     { $('#idModalInfoDetail').html("Nombre d'enregistrements droppés: <strong>"+Response.nbr_archive_deleted+"</strong>" );
       $('#idModalInfo').modal("show");
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Archive_testdb (  )
  { Send_to_API ( 'PUT', "/api/archive/testdb", null, function (Response)
     { $('#idModalInfoDetail').html("Etat du test de connexion: <strong>"+Response.result+"<br>" + Response.details + "</strong>" );
       $('#idModalInfo').modal("show");
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Archive_Del ( table_name )
  { var json_request = JSON.stringify( { table_name : table_name } );
    Send_to_API ( 'DELETE', "/api/archive/del", json_request, function ()
     { $('#idTableArchive').DataTable().ajax.reload(null, false);
     });
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Archive_Del ( table_name )
  { table = $('#idTableArchive').DataTable();
    selection = table.ajax.json().tables.filter( function(item) { return item.table_name==table_name } )[0];
    $('#idModalDelTitre').text ( "Détruire la table ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer cette table et ses enregistrements ?"+
                                 "<hr>"+
                                 "<strong>"+table_name + "<br>"+selection.table_rows+" enregistrements</strong>"
                                );
    $('#idModalDelValider').attr( "onclick", "Valider_Archive_Del('"+table_name+"')" );
    $('#idModalDel').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Archive_sauver_parametre ( )
  { var json_request =
     { hostname:    $('#idArchiveDBHostname').val(),
       port:        parseInt($('#idArchiveDBPort').val()),
       username:    $('#idArchiveDBUsername').val(),
       database:    $('#idArchiveDBDatabase').val(),
       retention:   parseInt($('#idArchiveDBRetention').val()),
       buffer_size: parseInt($('#idArchiveDBBufferSize').val()),
     };
    if ($('#idArchiveDBPassword').val().length > 0) { json_request.password = $('#idArchiveDBPassword').val(); }
    Send_to_API ( 'POST', "/api/archive/set", JSON.stringify(json_request), null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/archive/status", null, function ( Response )
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').slideUp(); }
                                  else { $('#idAlertThreadNotRunning').slideDown(); }
       $('#idArchiveDBHostname').val(Response.hostname);
       $('#idArchiveDBPort').val(Response.port);
       $('#idArchiveDBUsername').val(Response.username);
       $('#idArchiveDBDatabase').val(Response.database);
       $('#idArchiveDBRetention').val(Response.retention);
       $('#idArchiveDBBufferSize').val(Response.buffer_size);
     });

    $('#idTableArchive').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/archive/table_status",	type : "GET", dataSrc: "tables",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "table_name", "title":"Nom de la table", "className": "align-middle text-center" },
            { "data": "table_rows", "title":"Nombre d'enregistrements", "className": "align-middle text-center" },
            { "data": "update_time", "title":"Last Update", "className": "align-middle text-center" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "danger", "Supprimer la table", "Show_Modal_Archive_Del", item.table_name, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    Charger_une_courbe ( "idCourbeNbArchive", "SYS", "ARCH_REQUEST_NUMBER", "HOUR" );

  }
