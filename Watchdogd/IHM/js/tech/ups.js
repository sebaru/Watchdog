 document.addEventListener('DOMContentLoaded', Load_page, false);

/************************************ Demande de refresh **********************************************************************/
 function UPS_Refresh ( )
  { $('#idTableUPS').DataTable().ajax.reload(null, false);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function UPS_Disable ( id )
  { table = $('#idTableUPS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : false,
       uuid   : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       UPS_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function UPS_Enable ( id )
  { table = $('#idTableUPS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    var json_request =
     { enable : true,
       uuid   : selection.uuid,
       thread_tech_id: selection.thread_tech_id,
       id     : selection.id
     };

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       UPS_Refresh();
     }, null );
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function UPS_Set ( selection )
  { var json_request =
     { uuid:           $('#idTargetProcess').val(),
       thread_tech_id:        $('#idUPSTechID').val(),
       host:           $('#idUPSHost').val(),
       name:           $('#idUPSName').val(),
       admin_username: $('#idUPSAdminUsername').val(),
       admin_password: $('#idUPSAdminPassword').val(),
     };
    if (selection) json_request.id = parseInt(selection.id);                                            /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/process/config", JSON.stringify(json_request), function(Response)
     { if (selection && selection.uuid != json_request.uuid) Process_reload ( selection.uuid );/* Restart de l'ancien subprocess si uuid différent */
       Process_reload ( json_request.uuid );                                /* Dans tous les cas, restart du subprocess cible */
       UPS_Refresh();
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function UPS_Edit ( id )
  { table = $('#idTableUPS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=ups", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, selection.uuid );
    $('#idUPSTitre').text("Editer la connexion UPS " + selection.thread_tech_id);
    $('#idUPSTechID').val( selection.thread_tech_id ).off("input").on("input", function () { Controle_thread_tech_id( "idUPS", selection.thread_tech_id ); } );
    $('#idUPSHost').val( selection.host );
    $('#idUPSName').val( selection.name );
    $('#idUPSAdminUsername').val( selection.admin_username );
    $('#idUPSAdminPassword').val( selection.admin_password );
    $('#idUPSValider').off("click").on( "click", function () { UPS_Set(selection); } );
    $('#idUPSEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function UPS_Add ( )
  { $('#idUPSTitre').text("Ajouter un UPS");
    Select_from_api ( "idTargetProcess", "/api/process/list", "name=ups", "Process", "uuid", function (Response)
                        { return ( Response.instance ); }, null );
    $('#idUPSTechID').val("").off("input").on("input", function () { Controle_thread_tech_id( "idUPS", null ); } );
    $('#idUPSHost').val( "" );
    $('#idUPSName').val( "" );
    $('#idUPSAdminUsername').val( "" );
    $('#idUPSAdminPassword').val( "" );
    $('#idUPSValider').off("click").on( "click", function () { UPS_Set(null); } );
    $('#idUPSEdit').modal("show");
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function UPS_Del_Valider ( selection )
  { var json_request = { uuid : selection.uuid, thread_tech_id: selection.thread_tech_id };
    Send_to_API ( 'DELETE', "/api/process/config", JSON.stringify(json_request), function(Response)
     { Process_reload ( json_request.uuid );
       UPS_Refresh();
     }, null );
  }
/**************************************** Supprime une connexion meteo ********************************************************/
 function UPS_Del ( id )
  { table = $('#idTableUPS').DataTable();
    selection = table.ajax.json().config.filter( function(item) { return item.id==id } )[0];
    Show_modal_del ( "Supprimer la connexion "+selection.thread_tech_id,
                     "Etes-vous sûr de vouloir supprimer cette connexion ?",
                     selection.thread_tech_id + " - "+selection.name +"@"+ selection.host,
                     function () { UPS_Del_Valider( selection ) } ) ;
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableUPS').DataTable(
     { pageLength : 50,
       fixedHeader: true,
       rowId: "id",
       ajax: {	url : "/api/process/config", type : "GET", data: { name: "ups" }, dataSrc: "config",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       columns:
        [ { "data": "instance",   "title":"Instance",   "className": "align-middle text-center" },
           { "data": null, "title":"Enabled", "className": "align-middle text-center",
              "render": function (item)
               { if (item.enable==true)
                  { return( Bouton ( "success", "Désactiver l'UPS",
                                     "UPS_Disable", item.id, "Actif" ) );
                  }
                 else
                  { return( Bouton ( "outline-secondary", "Activer l'UPS",
                                     "UPS_Enable", item.id, "Désactivé" ) );
                  }
               },
           },
           { "data": null, "title":"Tech_id", "className": "align-middle text-center",
             "render": function (item)
               { return( Lien ( "/tech/dls_source/"+item.thread_tech_id, "Voir la source", item.thread_tech_id ) ); }
           },
           { "data": "description", "title":"Description", "className": "align-middle text-center " },
           { "data": "name", "title":"Name", "className": "align-middle text-center" },
           { "data": "host", "title":"Host", "className": "align-middle text-center" },
           { "data": "admin_username", "title":"Username", "className": "align-middle text-center" },
           { "data": "admin_password", "title":"Password", "className": "align-middle text-center" },
           { "data": null, "title":"IO_COMM", "className": "align-middle text-center",
             "render": function (item)
               { if (item.comm==true) { return( Bouton ( "success", "Comm OK", null, null, "1" ) );        }
                                 else { return( Bouton ( "outline-secondary", "Comm Failed", null, null, "0" ) ); }
               },
           },
           { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
             "render": function (item)
               { boutons = Bouton_actions_start ();
                 boutons += Bouton_actions_add ( "primary", "Editer la connexion", "UPS_Edit", item.id, "pen", null );
                 boutons += Bouton_actions_add ( "danger", "Supprimer la connexion", "UPS_Del", item.id, "trash", null );
                 boutons += Bouton_actions_end ();
                 return(boutons);
               },
           }
        ],
         /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
