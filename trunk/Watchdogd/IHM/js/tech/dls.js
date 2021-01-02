 document.addEventListener('DOMContentLoaded', Load_page, false);

 var compil_status = [ [ "Never compiled yet", "info" ],
                       [ "Database Export failed", "outline-danger" ],
                       [ "Error loading source file", "outline-danger" ],
                       [ "Error loading log file", "outline-danger" ],
                       [ "Syntax error", "outline-danger" ],
                       [ "Error Fork GCC", "outline-danger" ],
                       [ "Warnings", "outline-warning" ],
                       [ "OK", "success" ],
                       [ "Functions are missing<br>Need compiling again", "outline-danger" ],
                       [ "Error, plugin is setting bits he does not own", "outline-danger" ],
                       [ "Error", "outline-danger" ]
                     ];

 function Go_to_dls_status ()
  { Redirect ( "/tech/dls_status" );
  }

/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Dls_Compiler_tous_dls ( )
  { var json_request = JSON.stringify( { compil_all: true } );
    Send_to_API ( 'POST', "/api/dls/compil", json_request, function ()
     { $('#idTableDLS').DataTable().ajax.reload();
     }, null );
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
    $('#idModalDelTitre').text ( "Détruire le module ?" );
    $('#idModalDelMessage').html("Etes-vous sur de vouloir supprimer le module DLS "+
                                 "et toutes ses dépendances (Mnémoniques, ...) ?<hr>"+
                                 "<strong>"+selection.tech_id + " - " + selection.shortname + "</strong>" + "<br>" + selection.name
                                );
    $('#idModalDelValider').attr( "onclick", "Valider_Dls_Del('"+tech_id+"')" );
    $('#idModalDel').modal("show");
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
 function Synoptique_Dls_Add ( syn_id )
  { table = $('#idTableSyn').DataTable();
    selection = table.ajax.json().synoptiques.filter( function(item) { return item.id==syn_id } )[0];

    var json_request =
       { syn_id : selection.syn_id,
         name   : $('#idModalDlsAddDescription').val(),
         tech_id: $('#idModalDlsAddTechID').val().toUpperCase(),
       };

    Send_to_API ( "POST", "/api/dls/set", JSON.stringify(json_request), function(Response)
     { $('#idTableSyn').DataTable().ajax.reload(null, false);
     }, null );
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Dls_Add_controle_techid ( )
  { table = $('#idTableDLS').DataTable();
    if (table.ajax.json().plugins.filter ( function (item)
                                            { return ( item.tech_id == $('#idModalDlsAddTechID').val().toUpperCase() ); } ).length!=0)
     { $('#idModalDlsAddTechID').addClass("bg-danger");
       $('#idModalDlsAddValider').attr( "disabled", true );
     }
    else
     { $('#idModalDlsAddTechID').removeClass("bg-danger");
       $('#idModalDlsAddValider').attr( "disabled", false );
     }
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Dls_Add ( syn_id )
  { $('#idModalDlsAddTechID').val("").removeClass("bg-danger");
    $('#idModalDlsAddTechID').attr("oninput", "Dls_Add_controle_techid()" );
    $('#idModalDlsAddDescription').val("");
    $('#idModalDlsAddValider').attr( "onclick", "Dls_Add("+syn_id+")" );
    Send_to_API ( "GET", "/api/syn/list", null, function (Response)
     { $('#idModalDlsAddPage').empty();
       $.each ( Response.synoptiques, function ( i, item )
        { $('#idModalDlsAddPage').append("<option value='"+item.id+"'>"+item.page+" - "+htmlEncode(item.libelle)+"</option>"); } );
     }, null );
    $('#idModalDlsAdd').modal("show");
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableDLS').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         ajax: {	url : "/api/dls/list",	type : "GET", dataSrc: "plugins",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         columns:
          [ { "data": "ppage", "title":"PPage", "className": "align-middle  text-center" },
            { "data": "page", "title":"Page", "className": "align-middle  text-center" },
            { "data": null, "title":"Started", "className": "align-middle  text-center",
              "render": function (item)
                { if (item.actif==true)
                   { return( Bouton ( "success", "Désactiver le plugin",
                                      "Dls_stop_plugin", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le plugin",
                                      "Dls_start_plugin", item.tech_id, "Désactivé" ) );
                   }
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
            { "data": "nbr_compil", "title":"Nbr Compil", "className": "align-middle text-center " },
            { "data": "nbr_ligne", "title":"Nbr Lignes", "className": "align-middle text-center " },
            { "data": null, "title":"Actions", "orderable": false, "className": "align-middle",
              "render": function (item)
                { boutons = Bouton_actions_start ();
                  boutons += Bouton_actions_add ( "outline-primary", "Voir le code", "Redirect", "/tech/dls_source/"+item.tech_id, "code", null );
                  boutons += Bouton_actions_add ( "outline-primary", "Voir les mnemos", "Redirect", "/tech/mnemos/"+item.tech_id, "book", null );
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
