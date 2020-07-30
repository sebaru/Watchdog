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
/************************************ Envoi les infos de modifications synoptique *********************************************/
 function Valider_Dls_Del ( dls_id )
  { var xhr = new XMLHttpRequest;
    xhr.open('DELETE', "/api/dls/del/"+dls_id );
    xhr.onreadystatechange = function( )
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idTableDLS').DataTable().ajax.reload(null, false);
          $('#idToastStatus').toast('show');
        }
       else { Show_Error( xhr.statusText ); }
     };
    xhr.send();
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Show_Modal_Dls_Del ( dls_id )
  { table = $('#idTableDLS').DataTable();
    selection = table.ajax.json().plugins.filter( function(item) { return item.id==dls_id } )[0];
    $('#idModalDLSDelTitre').text ( "Détruire le module ?" );
    $('#idModalDLSDelMessage').html("Etes-vous sur de vouloir supprimer le module DLS "+
                                    "et toutes ses dépendances (Mnémoniques, ...) ?<hr>"+
                                    "<strong>"+selection.tech_id + " - " + selection.shortname + "</strong>" + "<br>" + selection.name
                                   );
    $('#idModalDLSDelValider').attr( "onclick", "Valider_Dls_Del("+dls_id+")" );
    $('#idModalDLSDel').modal("show");
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
          [ { "data": "ppage", "title":"PPage", "className": "align-middle hidden-xs text-center" },
            { "data": "page", "title":"Page", "className": "align-middle hidden-xs text-center" },
            { "data": null, "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) );
                },
              "title":"tech_id", "orderable": true
            },
            { "data": "package", "title":"Package", "className": "align-middle hidden-xs" },
            { "data": null, "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.shortname ) );
                },
              "title":"Nom court", "orderable": true
            },
            { "data": null, "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.name ) );
                },
              "title":"Libellé", "orderable": true
            },
            { "data": null, "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { if (item.actif==true)
                   { return( Bouton ( "success", "Désactiver le plugin",
                                      "Dls_disable_plugin", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le plugin",
                                      "Dls_enable_plugin", item.tech_id, "Désactivé" ) );
                   }
                },
              "title":"Started", "orderable": true
            },
            { "data": null, "className": "align-middle hidden-xs",
              "render": function (item)
                { return( Bouton ( compil_status[item.compil_status][1],
                                  "Statut de la compilation", null, null,
                                  compil_status[item.compil_status][0]
                                )
                        );
                },
              "title":"Compil", "orderable": true
            },
            { "data": "nbr_compil", "title":"Nbr Compil", "className": "align-middle text-center hidden-xs" },
            { "data": "nbr_ligne", "title":"Nbr Lignes", "className": "align-middle text-center hidden-xs" },
            { "data": null,
              "render": function (item)
                { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir le code' "+
                         "onclick=Redirect('/tech/dls_source/"+item.tech_id+"')>"+
                         "<i class='fas fa-code'></i></button>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir les mnemos' "+
                         "onclick=Redirect('/tech/mnemos/"+item.tech_id+"')>"+
                         "<i class='fas fa-book'></i></button>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir le RUN' "+
                         "onclick=Redirect('/tech/run/"+item.tech_id+"')>"+
                         "<i class='fas fa-eye'></i></button>"+
                         "<button class='btn btn-danger btn-block btn-sm' data-toggle='tooltip' title='Supprimer le plugin' "+
                         "onclick=Show_Modal_Dls_Del('"+item.id+"')>"+
                         "<i class='fas fa-trash'></i></button>"+
                         "</div>"
                        )
                },
              "title":"Actions", "orderable": false
            }
          ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

  }
