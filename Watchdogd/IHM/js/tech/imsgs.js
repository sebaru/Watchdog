 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function IMSGS_Refresh ( )
  { $('#idTableIMSGS').DataTable().ajax.reload(null, false);
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function IMSGS_Test ( id )
  { table = $('#idTableIMSGS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { thread_tech_id: selection.thread_tech_id,
       zmq_tag : "test"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function IMSGS_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableIMSGS').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function IMSGS_Del ( id )
  { table = $('#idTableIMSGS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.jabberid,
                     function () { IMSGS_Del_Valider( selection ) } ) ;
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function IMSGS_Set ( selection )
  { var json_request =
       { uuid    : $('#idTargetProcess').val(),
         thread_tech_id : $('#idIMSGSTechID').val(),
         description: $('#idIMSGSDescription').val(),
         jabberid: $('#idIMSGSJabberID').val(),
         password: $('#idIMSGSPassword').val(),
       };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableIMSGS').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function IMSGS_Edit ( id )
  { table = $('#idTableIMSGS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    $('#idIMSGSTitre').text("Editer la conexion " + selection.thread_tech_id);
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=imsgs", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idIMSGSTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idIMSGS", selection.thread_tech_id ); } );
    $('#idIMSGSDescription').val( selection.description );
    $('#idIMSGSJabberID').val( selection.jabberid );
    $('#idIMSGSPassword').val( selection.password );
    $('#idIMSGSValider').off("click").on( "click", function () { IMSGS_Set(selection); } );
    $('#idIMSGSEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function IMSGS_Add ( )
  { $('#idIMSGSTitre').text("Ajouter une connexion XMPP");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=imsgs", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idIMSGSTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idIMSGS", null ); } );
    $('#idIMSGSJabberID').val( "" );
    $('#idIMSGSPassword').val( "" );
    $('#idIMSGSValider').off("click").on( "click", function () { IMSGS_Set(null); } );
    $('#idIMSGSEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableIMSGS').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "imsgs" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle text-center " },
           { "data": "jabberid", "title":"JabberID", "className": "align-middle " },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "IMSGS_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "outline-primary", "Test IMSGS", "IMSGS_Test", item.id, "question", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "IMSGS_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
  }
