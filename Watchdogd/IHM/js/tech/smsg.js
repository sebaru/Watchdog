 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function SMSG_Refresh ( )
  { $('#idTableSMSG').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function SMSG_Set ( selection )
  { var json_request =
     { uuid:        $('#idTargetProcess').val(),
       thread_tech_id:     $('#idSMSGTechID').val(),
       description: $('#idSMSGDescription').val(),
       ovh_service_name:       $('#idSMSGOVHServiceName').val(),
       ovh_application_key:    $('#idSMSGOVHApplicationKey').val(),
       ovh_application_secret: $('#idSMSGOVHApplicationSecret').val(),
       ovh_consumer_key:       $('#idSMSGOVHConsumerKey').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       SMSG_Refresh();
     }, null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMSG_Test_GSM ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { thread_tech_id: selection.thread_tech_id,
       zmq_tag: "test_gsm"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMSG_Test_OVH ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { thread_tech_id: selection.thread_tech_id,
       zmq_tag : "test_ovh"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function SMSG_Edit ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=smsg", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idSMSGTitre').text("Editer la connexion GSM " + selection.thread_tech_id);
    $('#idSMSGTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idSMSG", selection.thread_tech_id ); } );
    $('#idSMSGDescription').val( selection.description );
    $('#idSMSGOVHServiceName').val( selection.ovh_service_name );
    $('#idSMSGOVHApplicationKey').val( selection.ovh_application_key );
    $('#idSMSGOVHApplicationSecret').val( selection.ovh_application_secret );
    $('#idSMSGOVHConsumerKey').val( selection.ovh_consumer_key );
    $('#idSMSGValider').off("click").on( "click", function () { SMSG_Set(selection); } );
    $('#idSMSGEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function SMSG_Add ( )
  { $('#idSMSGTitre').text("Ajouter un équipement GSM");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=smsg", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idSMSGTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idSMSG", null ); } );
    $('#idSMSGDescription').val("");
    $('#idSMSGOVHServiceName').val("");
    $('#idSMSGOVHApplicationKey').val("");
    $('#idSMSGOVHApplicationSecret').val("");
    $('#idSMSGOVHConsumerKey').val("");
    $('#idSMSGValider').off("click").on( "click", function () { SMSG_Set(null); } );
    $('#idSMSGEdit').modal("show");
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function SMSG_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       SMSG_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function SMSG_Del ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.description,
                     function () { SMSG_Del_Valider( selection ) } ) ;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableSMSG').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false, ordering: true, searching: true,
       ajax: { url : "/api/process/config", type : "GET", data: { name: "smsg" }, dataSrc: "config",
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
           { "data": "ovh_consumer_key", "title":"OVH Consumer Key", "className": "align-middle " },
           { "data": "ovh_service_name", "title":"OVH Service Name", "className": "align-middle " },
           { "data": "ovh_application_key", "title":"OVH App Key", "className": "align-middle " },
           { "data": "ovh_application_secret", "title":"OVH App Secret", "className": "align-middle " },
           { "data": "nbr_sms", "title":"#SMS", "className": "align-middle " },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "SMSG_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "outline-primary", "Test GSM", "SMSG_Test_GSM", item.id, "question", null );
                 boutons += Bouton_actions_add ( "outline-secondary", "Test OVG", "SMSG_Test_OVH", item.id, "question", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "SMSG_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
         ],
       /*order: [ [0, "desc"] ],*/
       responsive: true,
     });

  }
