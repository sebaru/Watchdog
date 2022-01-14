 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function AUDIO_Refresh ( )
  { $('#idTableAUDIO').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function AUDIO_Set ( selection )
  { var json_request =
     { uuid:     $('#idTargetProcess').val(),
       thread_tech_id : $('#idAUDIOTechID').val(),
       language: $('#idAUDIOLanguage').val(),
       device  : $('#idAUDIODevice').val(),
       description: $('#idAUDIODescription').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableAUDIO').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function AUDIO_Test ( id )
  { table = $('#idTableAUDIO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { thread_tech_id: selection.thread_tech_id,
       zmq_tag: "test"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function AUDIO_Edit ( id )
  { table = $('#idTableAUDIO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=audio", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idAUDIOTitre').text("Editer la connexion GSM " + selection.thread_tech_id);
    $('#idAUDIOTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idAUDIO", selection.thread_tech_id ); } );
    $('#idAUDIOLanguage').val( selection.language );
    $('#idAUDIODevice').val( selection.device );
    $('#idAUDIODescription').val( selection.description );
    $('#idAUDIOValider').off("click").on( "click", function () { AUDIO_Set(selection); } );
    $('#idAUDIOEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function AUDIO_Add ( )
  { $('#idAUDIOTitre').text("Ajouter une zone AUDIO");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=audio", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idAUDIOTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idAUDIO", null ); } );
    $('#idAUDIOLanguage').val( "" );
    $('#idAUDIODevice').val( "" );
    $('#idAUDIODescription').val( "" );
    $('#idAUDIOValider').off("click").on( "click", function () { AUDIO_Set(null); } );
    $('#idAUDIOEdit').modal("show");
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function AUDIO_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       $('#idTableAUDIO').DataTable().ajax.reload(null, false);
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function AUDIO_Del ( id )
  { table = $('#idTableAUDIO').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la zone de diffusion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette zone de diffusion ?",
                     selection.thread_tech_id + " - "+selection.description,
                     function () { AUDIO_Del_Valider( selection ) } ) ;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableAUDIO').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "audio" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       columns:
         [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle " },
           { "data": "language", "title":"Language", "className": "align-middle " },
           { "data": "device", "title":"Device", "className": "align-middle " },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la zone de difufsion", "AUDIO_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "outline-primary", "Tester la diffusion", "AUDIO_Test", item.id, "question", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la zone", "AUDIO_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
