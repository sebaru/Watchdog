 document.addEventListener('DOMContentLoaded', Load_page, false);

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
    $('#idModalDLSDelTitre').text ( "Détruire le module ?" );
    $('#idModalDLSDelMessage').html("Etes-vous sur de vouloir supprimer le module DLS "+
                                    "et toutes ses dépendances (Mnémoniques, ...) ?<hr>"+
                                    "<strong>"+table.cell( "#"+dls_id, "tech_id:name" ).data()+" - "+
                                    table.cell( "#"+dls_id, "shortname:name" ).data()+
                                    "</strong>" );
    $('#idModalDLSDelValider').attr( "onclick", "Valider_Dls_Del("+dls_id+")" );
    $('#idModalDLSDel').modal("show");
  }

/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Go_to_dls_source_edit ( tech_id )
  { window.location = "/tech/dls_source/"+tech_id;
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Go_to_dls_mnemos ( tech_id )
  { window.location = "/tech/mnemos/"+tech_id;
  }
/********************************************* Afichage du modal d'edition synoptique *****************************************/
 function Go_to_dls_run ( tech_id )
  { window.location = "/tech/run/"+tech_id;
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
          [ { "data": "id", "title":"#", "className": "text-center" },
            { "data": "ppage", "title":"PPage", "className": "hidden-xs text-center" },
            { "data": "page", "title":"Page", "className": "hidden-xs text-center" },
            { "data": "tech_id", "title":"TechID", "name":"tech_id", "className": "text-center" },
            { "data": "package", "title":"Package", "className": "hidden-xs" },
            { "data": "shortname", "title":"Nom court", "name":"shortname", "className": "text-center" },
            { "data": "name", "title":"Libellé", "className": "hidden-xs" },
            { "data": "actif", "title":"Enable", "className": "hidden-xs" },
            { "data": "compil_status", "title":"Compil_status", "className": "hidden-xs" },
            { "data": "nbr_compil", "title":"Nbr Compil", "className": "hidden-xs" },
            { "data": "nbr_ligne", "title":"Nbr Ligne", "className": "hidden-xs" },
            { "data": null,
              "render": function (item)
                { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir le code' "+
                         "onclick=Go_to_dls_source_edit('"+item.tech_id+"')>"+
                         "<i class='fas fa-code'></i></button>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir les mnemos' "+
                         "onclick=Go_to_dls_mnemos('"+item.tech_id+"')>"+
                         "<i class='fas fa-book'></i></button>"+
                         "<button class='btn btn-outline-primary btn-sm' data-toggle='tooltip' title='Voir le RUN' "+
                         "onclick=Go_to_dls_run('"+item.tech_id+"')>"+
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
