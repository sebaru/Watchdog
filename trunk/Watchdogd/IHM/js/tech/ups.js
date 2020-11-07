 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Ups_refresh ( )
  { $('#idTableUps').DataTable().ajax.reload(null, false);
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Ups_enable_module ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/ups/start", json_request, function ()
     { $('#idTableUps').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Ups_disable_module ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/process/ups/stop", json_request, function ()
     { $('#idTableUps').DataTable().ajax.reload(null, false);
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Ups_Del ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'DELETE', "/api/process/ups/del", json_request, function ()
     { $('#idTableUps').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Ups_Del ( tech_id )
  { table = $('#idTableUps').DataTable();
    selection = table.ajax.json().ups.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalDelTitre').text ( "Détruire l'onduleur ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer cet onduleur ?"+
                                 "<hr>"+
                                 "<strong>"+selection.tech_id + "</strong> - " + selection.name + "@" + selection.host
                                );
    $('#idModalDelValider').attr( "onclick", "Valider_Ups_Del('"+tech_id+"')" );
    $('#idModalDel').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Ups_Edit ( tech_id )
  { var json_request = JSON.stringify(
       { tech_id : $('#idModalUpsEditTechID').val(),
         host: $('#idModalUpsEditHost').val(),
         name: $('#idModalUpsEditName').val(),
         admin_username: $('#idModalUpsEditAdminUsername').val(),
         admin_password: $('#idModalUpsEditAdminPassword').val(),
       }
     );
    Send_to_API ( 'POST', "/api/process/ups/set", json_request, function ()
     { $('#idTableUps').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Ups_Edit ( tech_id )
  { table = $('#idTableUps').DataTable();
    selection = table.ajax.json().ups.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalUpsEditTitre').text ( "Editer UPS - " + selection.tech_id );
    $('#idModalUpsEditTechID').val ( selection.tech_id );
    $('#idModalUpsEditTechID').attr ( "readonly", true );
    $('#idModalUpsEditHost').val ( selection.host );
    $('#idModalUpsEditName').val ( selection.name );
    $('#idModalUpsEditAdminUsername').val( selection.admin_username );
    $('#idModalUpsEditAdminPassword').val( selection.admin_password );
    $('#idModalUpsEditValider').attr( "onclick", "Valider_Ups_Edit('"+tech_id+"')" );
    $('#idModalUpsEdit').modal("show");
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Ups_Add ( )
  { var json_request = JSON.stringify(
       { tech_id: $('#idModalUpsEditTechID').val().toUpperCase(),
         host: $('#idModalUpsEditHost').val(),
         name: $('#idModalUpsEditName').val(),
         admin_username: $('#idModalUpsEditAdminUsername').val(),
         admin_password: $('#idModalUpsEditAdminPassword').val(),
       }
     );
    Send_to_API ( 'POST', "/api/process/ups/add", json_request, function ()
     { $('#idTableUps').DataTable().ajax.reload(null, false);
     });
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Ups_Add ()
  { $('#idModalUpsEditTitre').text ( "Ajouter un composant WAGO" );
    $('#idModalUpsEditTechID').val ( "" );
    $('#idModalUpsEditTechID').attr ( "readonly", false );
    $('#idModalUpsEditHost').val ( "" );
    $('#idModalUpsEditName').val ( "" );
    $('#idModalUpsEditAdminUsername').val( "" );
    $('#idModalUpsEditAdminPassword').val( "" );
    $('#idModalUpsEditValider').attr( "onclick", "Valider_Ups_Add()" );
    $('#idModalUpsEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/process/ups/status", null, function(Response)
     { if (Response.thread_is_running) { $('#idAlertThreadNotRunning').hide(); }
                                  else { $('#idAlertThreadNotRunning').show(); }
     });
    $('#idTableUps').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/process/ups/list",	type : "GET", dataSrc: "ups",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": null, "title":"TechID", "className": "align-middle text-center",
              "render": function (item)
                { return('<a href=/tech/dls_run/'+item.tech_id+'>'+item.tech_id+'</a>'); }
            },
            { "data": null, "title":"Enable", "className": "align-middle text-center",
              "render": function (item)
                { if (item.enable==false)
                       { return( Bouton ( "outline-secondary", "Activer cet UPS", "Ups_enable_module", item.tech_id, "Désactivé" ) ); }
                  else { return( Bouton ( "success", "Désactiver cet UPS", "Ups_disable_module", item.tech_id, "Actif" ) ); }
                }
            },
            { "data": null, "title":"Started", "className": "align-middle text-center",
              "render": function (item)
                { if (item.started==false)
                       { return( Bouton ( "outline-warning", "Cet UPS ne tourne pas", null, null, "Disconnected" ) ); }
                  else { return( Bouton ( "success", "Cet UPS est activé", null, null, "Connected" ) ); }
                }
            },
            { "data": "name", "title":"Name", "className": "align-middle text-center" },
            { "data": "host", "title":"Host", "className": "align-middle text-center" },
            { "data": "description", "title":"Description", "className": "align-middle text-center" },
            { "data": "admin_username", "title":"Username", "className": "align-middle text-center" },
            { "data": "admin_password", "title":"Password", "className": "align-middle text-center" },
            { "data": "nbr_connexion", "title":"Nbr Connexion", "className": "align-middle text-center" },
            { "data": "date_create", "title":"Date Création", "className": "align-middle text-center hidden-xs" },
            { "data": null, "title":"Actions", "orderable": false, "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Editer cet UPS", "Show_Modal_Ups_Edit", item.tech_id, "pen", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet UPS", "Show_Modal_Ups_Del", item.tech_id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                },
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
