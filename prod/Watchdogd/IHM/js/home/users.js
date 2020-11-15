 document.addEventListener('DOMContentLoaded', Load_page, false);
 var Instances;

/******************************************************************************************************************************/
 function Mnemos_CI_set_archivage ( acronyme )
  { table = $('#idTableCptImp').DataTable();
    selection = table.ajax.json().CI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "CI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: $('#idCIArchivage'+acronyme).val()
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableCptImp').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_R_set_archivage ( acronyme )
  { table = $('#idTableRegistre').DataTable();
    selection = table.ajax.json().R.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "R",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: $('#idRArchivage'+acronyme).val()
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableRegistre').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_MSG_set ( acronyme )
  { table = $('#idTableMessage').DataTable();
    selection = table.ajax.json().MSG.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "MSG",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         sms        : $('#idMSGSms'+acronyme).val(),
         libelle_sms: $('#idMSGLibelleSms'+acronyme).val(),
         profil_audio : $('#idMSGProfilAudio'+acronyme).val(),
         libelle_audio: $('#idMSGLibelleAudio'+acronyme).val(),
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableMessage').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_DI_set ( acronyme )
  { table = $('#idTableEntreeTor').DataTable();
    selection = table.ajax.json().DI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "DI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         etat     : true
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, null, null );
  }
/******************************************************************************************************************************/
 function Mnemos_DI_reset ( acronyme )
  { table = $('#idTableEntreeTor').DataTable();
    selection = table.ajax.json().DI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "DI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         etat     : false
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, null, null );
  }
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
          [ { "data": "username",   "title":"Username",   "className": "align-middle text-center" },
            { "data": null, "title":"Enable", "className": "align-middle hidden-xs text-center",
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
            { "data": null, "title":"Level", "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { return( Select_Access_level ( "idUserAccessLevel_"+item.username,
                                                "User_change('"+item.username+"')",
                                                item.access_level )
                        );
                }
            },
            { "data": null, "title":"Notification", "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { if (item.sms_enable==true)
                   { return( Bouton ( "success", "Désactiver les notifications",
                                      "User_disable_notif", item.username, "Oui" ) );
                   }
                  else
                   { return( Bouton ( "outline-warning", "Activer les notiications",
                                      "User_enable_notif", item.username, "Désactivé" ) );
                   }
                }
            },            { "data": null, "title":"Adresse Mail", "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Input ( "idUserMail_"+item.username,
                                  "User_change('"+item.username+"')",
                                  "Adresse de messagerie",
                                  item.email )
                        );
                }
            },
            { "data": null, "title":"Messagerie Instantanée", "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Input ( "idUserJabberID_"+item.username,
                                  "User_change('"+item.username+"')",
                                  "Adresse de messagerie instantanée",
                                  item.imsg_jabberid )
                        );
                }
            },
            { "data": null, "title":"Téléphone", "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Input ( "idUserPhone_"+item.username,
                                  "User_change('"+item.username+"')",
                                  "Téléphone de cet utilisateur",
                                  item.sms_phone )
                        );
                }
            },
            { "data": null, "title":"Commentaire", "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Input ( "idUserComment_"+item.username,
                                  "User_change('"+item.username+"')",
                                  "Qui est cet utilisateur ?",
                                  item.comment )
                        );
                }
            },
            { "data": null, "title":"Actions", "orderable": false, "className":"align-middle text-center",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "warning", "Reseter son mot de passe", "User_password_reset", item.username, "key", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer cet utilisateur", "Show_Modal_User_Del", item.username, "trash", null );
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
