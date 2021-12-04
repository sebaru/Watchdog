 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande l'envoi d'un SMSG de test ******************************************************/
 function SMSG_Refresh ( )
  { $('#idTableSMSG').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function SMSG_Set ( selection )
  { var json_request =
     { uuid:        $('#idTargetInstance').val(),
       tech_id:     $('#idSMSGTechID').val(),
       description: $('#idSMSGDescription').val(),
       ovh_service_name:       $('#idSMSGOVHServiceName').val(),
       ovh_application_key:    $('#idSMSGOVHApplicationKey').val(),
       ovh_application_secret: $('#idSMSGOVHApplicationSecret').val(),
       ovh_consumer_key:       $('#idSMSGOVHConsumerKey').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection.uuid != json_request.uuid) Process_reload ( selection.uuid ); /* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       $('#idTableSMSG').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMSG_Test_GSM ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { tech_id: selection.tech_id,
       zmq_tag: "test_gsm"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/************************************ Demande l'envoi d'un SMS de test ********************************************************/
 function SMSG_Test_OVH ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { tech_id: selection.tech_id,
       zmq_tag : "test_ovh"
     };
    Send_to_API ( 'POST', "/api/process/send", JSON.stringify(json_request), null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function SMSG_Edit ( id )
  { table = $('#idTableSMSG').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=smsg", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idSMSGTitre').text("Editer la source Météo " + selection.tech_id);
    $('#idSMSGTechID').val( selection.tech_id ).off("input").on("input", function () { Controle_tech_id( "idSMSG", null ); } );
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
    Select_from_api ( "idTargetInstance", "/api/process/list", "name=smsg", "Process", "uuid", function (Response)
                        { return ( Response.host ); }, null );
    $('#idSMSGTechID').val("").off("input").on("input", function () { Controle_tech_id( "idSMSG", null ); } );
    $('#idSMSGDescription').val("");
    $('#idSMSGOVHServiceName').val("");
    $('#idSMSGOVHApplicationKey').val("");
    $('#idSMSGOVHApplicationSecret').val("");
    $('#idSMSGOVHConsumerKey').val("");
    $('#idSMSGValider').off("click").on( "click", function () { SMSG_Set(selection); } );
    $('#idSMSGEdit').modal("show");
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
         [ { "data": "host",   "title":"Host",   "className": "align-middle text-center" },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle " },
           { "data": "ovh_consumer_key", "title":"OVH Consumer Key", "className": "align-middle " },
           { "data": "ovh_service_name", "title":"OVH Service Name", "className": "align-middle " },
           { "data": "ovh_application_key", "title":"OVH App Key", "className": "align-middle " },
           { "data": "ovh_application_secret", "title":"OVH App Secret", "className": "align-middle " },
           { "data": "nbr_sms", "title":"#SMS", "className": "align-middle " },
           { "data": null, "title":"comm", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
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
