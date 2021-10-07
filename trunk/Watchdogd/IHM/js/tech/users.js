 document.addEventListener('DOMContentLoaded', Load_page, false);

/******************************************************************************************************************************/
 function User_disable_user ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username    : selection.username,
         enable      : false,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_enable_user ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username    : selection.username,
         enable      : true,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_reset_password ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username      : selection.username,
         reset_password: true,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_enable_cde ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username : selection.username,
         allow_cde: true,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_disable_cde ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username : selection.username,
         allow_cde: false,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_enable_notif ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username     : selection.username,
         notification : true,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_disable_notif ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username     : selection.username,
         notification : false,
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function User_set ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username    : selection.username,
         access_level: parseInt($('#idUserLevel_'+username).val()),
         email       : $('#idUserMail_'+username).val(),
         xmpp        : $('#idUserXmpp_'+username).val(),
         phone       : $('#idUserPhone_'+username).val(),
         comment     : $('#idUserComment_'+username).val(),
       }
     );

    Send_to_API ( 'POST', "/api/users/set", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_user_del ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    Show_modal_del ( "Supprimer cet utilisateur ?",
                     "Etes-vous sur de vouloir supprimer l'utilisateur "+selection.username+ "?",
                      selection.username + " - " + selection.comment,
                     "User_Valider_user_del('"+username+"')" );
  }
/******************************************************************************************************************************/
 function User_Valider_user_del ( username )
  { table = $('#idTableUsers').DataTable();
    selection = table.ajax.json().users.filter( function(item) { return (item.username==username) } )[0];
    var json_request = JSON.stringify(
       { username    : selection.username,
       }
     );

    Send_to_API ( 'DELETE', "/api/users/del", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function Users_Valider_add_user ()
  { var json_request = JSON.stringify(
       { username    : $('#idModalUserNewUsername').val(),
         password    : $('#idModalUserNewPassword').val(),
         email       : $('#idModalUserNewEmail').val(),
       }
     );

    Send_to_API ( 'POST', "/api/users/add", json_request, function ()
     { $('#idTableUsers').DataTable().ajax.reload(null, false);
     }, null);
  }
/******************************************************************************************************************************/
 function Users_Show_add_user ()
  { $('#idModalUserAdd').modal("show"); }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");
    $('#idTableUsers').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/users/list",	type : "GET", data: { }, dataSrc: "users",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
          [ { "data": "username",   "title":"Username", "className": "align-middle text-center" },
            { "data": null, "title":"Enable", "className": "align-middle  text-center",
              "render": function (item)
                { if (item.enable==true)
                   { return( Bouton ( "success", "Désactiver cet utilisateur",
                                      "User_disable_user", item.username, "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-warning", "Activer cet utilisateur",
                                      "User_enable_user", item.username, "Désactivé" ) );
                   }
                }
            },
            { "data": null, "title":"Level", "className": "align-middle  text-center",
              "render": function (item)
                { return( Select_Access_level ( "idUserLevel_"+item.username,
                                                "User_set('"+item.username+"')",
                                                item.access_level )
                        );
                }
            },
            { "data": null, "title":"Notification/Cde", "className": "align-middle  text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  if (item.notification==true)
                   { boutons += Bouton_actions_add ( "success", "Désactiver les notifications",
                                                     "User_disable_notif", item.username, "bell", null );
                   }
                  else
                   { boutons += Bouton_actions_add ( "outline-warning", "Activer les notifications",
                                                     "User_enable_notif", item.username, "bell-slash", null );
                   }
                  if (item.allow_cde==true)
                   { boutons += Bouton_actions_add ( "success", "Interdire les commandes",
                                                     "User_disable_cde", item.username, "phone", null );
                   }
                  else
                   { boutons += Bouton_actions_add ( "outline-warning", "Autoriser les commandes",
                                                     "User_enable_cde", item.username, "phone-slash", null );
                   }
                  boutons += Bouton_actions_end ();
                  return(boutons);
                }
            },
            { "data": null, "title":"Adresse Mail", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "email", "idUserMail_"+item.username,
                                  "User_set('"+item.username+"')",
                                  "Adresse de messagerie",
                                  item.email )
                        );
                }
            },
            { "data": null, "title":"Messagerie Instantanée", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "email", "idUserXmpp_"+item.username,
                                  "User_set('"+item.username+"')",
                                  "Adresse de messagerie instantanée",
                                  item.xmpp )
                        );
                }
            },
            { "data": null, "title":"Téléphone", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "tel", "idUserPhone_"+item.username,
                                  "User_set('"+item.username+"')",
                                  "Téléphone de cet utilisateur",
                                  item.phone )
                        );
                }
            },
            { "data": null, "title":"Commentaire", "className": "align-middle ",
              "render": function (item)
                { return( Input ( "text", "idUserComment_"+item.username,
                                  "User_set('"+item.username+"')",
                                  "Qui est cet utilisateur ?",
                                  item.comment )
                        );
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "warning", "Reseter son mot de passe", "User_reset_password", item.username, "key", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet utilisateur", "Show_Modal_user_del", item.username, "trash", null );
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
