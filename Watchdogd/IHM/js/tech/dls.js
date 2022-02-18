 document.addEventListener('DOMContentLoaded', Load_page, false);

 var compil_status = [ [ "Never compiled yet", "info" ],
                       [ "Database Export failed", "outline-danger" ],
                       [ "Error loading source file", "outline-danger" ],
                       [ "Error loading log file", "outline-danger" ],
                       [ "Syntax error", "outline-danger" ],
                       [ "Error Fork GCC", "outline-danger" ],
                       [ "OK", "success" ],
                       [ "Warnings", "outline-warning" ],
                     ];

 function Go_to_dls_status ()
  { Redirect ( "/tech/dls_status" );
  }

/******************************************************************************************************************************/
 function Dls_start_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/start", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload();
     }
    );
  }
 function Dls_stop_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/stop", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload();
     }
    );
  }
 function Dls_debug_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/debug", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload();
     });
  }

 function Dls_undebug_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/undebug", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload();
     });
  }
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Dls_Del ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'DELETE', "/api/dls/del", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload(null, false);
     }
    );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Dls_Del ( tech_id )
  { table = $('#idTableDLS').DataTable();
    selection = table.ajax.json().plugins.filter( function(item) { return item.tech_id==tech_id } )[0];
    if (selection.dls_id==1)
     { Show_Error ( "La suppression du DLS originel est interdite !" ); return; }

    Show_modal_del ( "Détruire le module ?",
                     "Etes-vous sur de vouloir supprimer le module DLS et toutes ses dépendances (Mnémoniques, ...) ?",
                     selection.tech_id + " - " + selection.shortname + " - " + selection.name,
                     function () { Valider_Dls_Del(tech_id); } );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Dls_compiler ( tech_id )
  { var json_request = JSON.stringify(
     { tech_id : tech_id,
     });
    Send_to_API ( "POST", "/api/dls/compil", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload(null, false);
     }, null);
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Dls_Set ( dls_id )
  { var json_request =
       { syn_id    : parseInt($('#idModalDlsEditPage').val()),
         shortname : $('#idModalDlsEditShortname').val(),
         name      : $('#idModalDlsEditDescription').val(),
         tech_id   : $('#idModalDlsEditTechID').val().toUpperCase(),
       };
    if (dls_id>0) json_request.dls_id = dls_id;                                                         /* Ajout ou édition ? */

    Send_to_API ( "POST", "/api/dls/set", JSON.stringify(json_request), function(Response)
     { $('#idTableDLS').DataTable().ajax.reload(null, false);
     }, null );
  }
/************************************ Controle de saisie avant envoi **********************************************************/
 function Dls_Set_controle_techid ( tech_id_initial )
  { FormatPage = RegExp(/^[a-zA-Z0-9_\.]+$/);
    table = $('#idTableDLS').DataTable();
    input = $('#idModalDlsEditTechID');

    if ( FormatPage.test(input.val())==false )
     { input.addClass("bg-danger");    $('#idModalDlsEditValider').attr("disabled", true);
       Popover_show ( input, 'Caractères autorisés', 'lettres, chiffres, _ et .' );
     }
    else if ( (table.ajax.json().plugins.filter( function(item)                                   /* Si tech_id deja existant */
                                               { return item.tech_id.toUpperCase()==input.val().toUpperCase() } )[0] !== undefined &&
              (tech_id_initial == null || input.val().toUpperCase() != tech_id_initial.toUpperCase()) )
       )
     { input.addClass("bg-danger");    $('#idModalDlsEditValider').attr("disabled", true);
       Popover_show ( input, 'Erreur !', 'Ce nom est déjà pris' );
     }
    else
     { if (tech_id_initial !== null && input.val().toUpperCase() != tech_id_initial.toUpperCase())
        { $('#idModalDlsEditValider').addClass("btn-danger").removeClass("btn-primary").text("Tout Recompiler"); }
       else
        { $('#idModalDlsEditValider').addClass("btn-primary").removeClass("btn-danger").text("Valider"); }
       input.removeClass("bg-danger"); $('#idModalDlsEditValider').attr("disabled", false);
       Popover_hide(input);
     }
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Dls_Add ( )
  { $('#idModalDlsEditTitre').text("Ajouter un D.L.S");
    $('#idModalDlsEditTechID').attr("oninput", "Dls_Set_controle_techid(null)" );
    $('#idModalDlsEditTechID').val("");
    Dls_Set_controle_techid ( null );
    $('#idModalDlsEditShortname').val("");
    $('#idModalDlsEditDescription').val("");
    $('#idModalDlsEditValider').attr( "onclick", "Dls_Set(0)" );
    Send_to_API ( "GET", "/api/syn/list", null, function (Response)
     { $('#idModalDlsEditPage').empty();
       $.each ( Response.synoptiques, function ( i, item )
        { $('#idModalDlsEditPage').append("<option value='"+item.syn_id+"'>"+item.page+" - "+htmlEncode(item.libelle)+"</option>"); } );
     }, null );
    $('#idModalDlsEdit').modal("show");
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Dls_Edit ( tech_id )
  { table = $('#idTableDLS').DataTable();
    selection = table.ajax.json().plugins.filter( function(item) { return item.tech_id==tech_id } )[0];
    $('#idModalDlsEditTitre').text("Modifier le D.L.S " + selection.tech_id );
    $('#idModalDlsEditTechID').val(selection.tech_id);
    $('#idModalDlsEditTechID').attr("oninput", "Dls_Set_controle_techid('"+tech_id+"')" );
    Dls_Set_controle_techid ( tech_id );
    $('#idModalDlsEditShortname').val(selection.shortname);
    $('#idModalDlsEditDescription').val(selection.name);
    $('#idModalDlsEditValider').attr( "onclick", "Dls_Set("+selection.dls_id+")" );
    Send_to_API ( "GET", "/api/syn/list", null, function (Response)
     { $('#idModalDlsEditPage').empty();
       $.each ( Response.synoptiques, function ( i, item )
        { $('#idModalDlsEditPage').append("<option value='"+item.syn_id+"'>"+item.page+" - "+htmlEncode(item.libelle)+"</option>"); } );
       $('#idModalDlsEditPage').val(selection.syn_id);
     }, null );
    $('#idModalDlsEdit').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableDLS').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "dls_id",
         ajax: {	url : "/api/dls/list",	type : "GET", dataSrc: "plugins",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "dls_id", "title":"#ID", "className": "align-middle  text-center" },
            { "data": "page", "title":"Page", "className": "align-middle  text-center" },
            { "data": null, "title":"Started", "className": "align-middle  text-center",
              "render": function (item)
                { if (item.actif==true)
                   { return( Bouton ( "success", "Désactiver le plugin",
                                      "Dls_stop_plugin", item.tech_id, "Actif" ) );
                   }
                  if (item.compil_status>=6) /* Si compil OK ou warning */
                   { return( Bouton ( "outline-secondary", "Activer le plugin",
                                      "Dls_start_plugin", item.tech_id, "Désactivé" ) );
                   }
                  return( Bouton ( "outline-secondary", "Compilation nécéssaire",
                                   null, null, "Désactivé" ) );
                }
            },
            { "data": null, title:"Debug",  "className": "text-center align-middle", "render": function (item)
              { if (item.debug==true)
                 { return( Bouton ( "warning", "Désactiver le debug", "Dls_undebug_plugin", item.tech_id, "Actif" ) ); }
                else
                 { return( Bouton ( "outline-secondary", "Activer le débug", "Dls_debug_plugin", item.tech_id, "Désactivé" ) ); }
              }
            },
            { "data": null, "title":"TechID", "className": "align-middle text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) );
                }
            },
            { "data": null, "title":"Nom court", "className": "align-middle",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.shortname ) );
                }
            },
            { "data": null, "title":"Libellé", "className": "align-middle ",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.name ) );
                }
            },
            { "data": null, "title":"Compil", "className": "align-middle ",
              "render": function (item)
                { return( Bouton ( compil_status[item.compil_status][1],
                                  "Statut de la compilation", null, null,
                                   compil_status[item.compil_status][0]
                                )
                        );
                }
            },
            { "data": null, "title":"Stats", "className": "align-middle text-center",
              "render": function (item)
                { return( Badge ( "primary", "Nombre de compilation", item.nbr_compil ) + "<br>" +
                          Badge ( "secondary", "Nombre de ligne", item.nbr_ligne ) );
                }
            },
            { "data": "compil_date", "title":"Date Compil", "className": "align-middle text-center " },
            { "data": null, "title":"Actions", "orderable": false, "className": "align-middle",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Voir le code", "Redirect", "/tech/dls_source/"+item.tech_id, "code", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Voir les mnemos", "Redirect", "/tech/mnemos/"+item.tech_id, "book", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Editer", "Show_Modal_Dls_Edit", item.tech_id, "pen", null );
                  boutons += Bouton_actions_add ( "outline-success", "Compiler le module", "Dls_compiler", item.tech_id, "coffee", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Voir les RUN", "Redirect", "/tech/dls_run/"+item.tech_id, "eye", null );
                  boutons += Bouton_actions_add ( "danger", "Supprimer le plugin", "Show_Modal_Dls_Del", item.tech_id, "trash", null );
                  boutons += Bouton_actions_end ();
                  return(boutons);
                }
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
