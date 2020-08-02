 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Go_to_source ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_source/"+vars[3] );
  }

 function Go_to_mnemos ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/mnemos/"+vars[3] );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log ("in load page !");
    $('#idTitle').html(vars[3]);

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/dls/run/"+vars[3], true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       $('#idTableEntreeTor').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.DI,
            rowId: "id",
            columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                       { "data": "libelle",    "title":"Libellé",    "className": "" },
                       { "data": "src_host",   "title":"src_host",   "className": "hidden-xs" },
                       { "data": "src_thread", "title":"src_thread", "className": "hidden-xs" },
                       { "data": "src_text",   "title":"src_text",   "className": "hidden-xs" },
                       { "data": null,
                         "render": function (item)
                          { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                   "    <button class='btn btn-outline-primary btn-sm' "+
                                               "onclick=window.location.href='atelier/"+item.id+"' "+
                                               "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                               "<i class='fas fa-image'></i></button>"+
                                   "    <button class='btn btn-danger btn-sm' "+
                                               "onclick=Show_Modal_Del("+item.acronyme+") "+
                                               "data-toggle='tooltip' title='Supprimer le mnémonique'>"+
                                               "<i class='fas fa-trash'></i></button>"+
                                   "</div>"
                                  )
                           },
                         "title":"Actions", "orderable": false, "className":"text-center"
                       }
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableEntreeANA').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.AI,
               rowId: "id",
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                       { "data": "type",   "title":"Type",   "className": "text-center hidden-xs" },
                       { "data": "valeur", "title":"Valeur", "className": "text-center hidden-xs" },
                       { "data": "valeur_brute", "title":"Valeur brute", "className": "text-center hidden-xs" },
                       { "data": "valeur_min", "title":"min", "className": "text-center hidden-xs" },
                       { "data": "valeur_max", "title":"max", "className": "text-center hidden-xs" },
                       { "data": "in_range", "title":"In_Range", "className": "text-center hidden-xs" },
                       { "data": "last_arch", "title":"last_arch", "className": "text-center hidden-xs" },
                    /*   { "data": null,
                         "render": function (item)
                          { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                   "    <button class='btn btn-outline-primary btn-sm' "+
                                               "onclick=window.location.href='atelier/"+item.id+"' "+
                                               "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                               "<i class='fas fa-image'></i></button>"+
                                   "    <button class='btn btn-danger btn-sm' "+
                                               "onclick=Show_Modal_Del("+item.acronyme+") "+
                                               "data-toggle='tooltip' title='Supprimer le mnémonique'>"+
                                               "<i class='fas fa-trash'></i></button>"+
                                   "</div>"
                                  )
                           },
                         "title":"Actions", "orderable": false, "className":"text-center"
                       }*/
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableSortieTor').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.DO,
            rowId: "id",
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                       { "data": "libelle",    "title":"Libellé",    "className": "" },
                       { "data": "dst_host",   "title":"dst_host",   "className": "hidden-xs" },
                       { "data": "dst_thread", "title":"dst_thread", "className": "hidden-xs" },
                       { "data": "dst_tag",    "title":"dst_tag",    "className": "hidden-xs" },
                       { "data": "dst_param1", "title":"dst_param1", "className": "hidden-xs" },
                       { "data": null,
                         "render": function (item)
                          { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                   "    <button class='btn btn-outline-primary btn-sm' "+
                                               "onclick=window.location.href='atelier/"+item.id+"' "+
                                               "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                               "<i class='fas fa-image'></i></button>"+
                                   "    <button class='btn btn-danger btn-sm' "+
                                               "onclick=Show_Modal_Del("+item.acronyme+") "+
                                               "data-toggle='tooltip' title='Supprimer le mnémonique'>"+
                                               "<i class='fas fa-trash'></i></button>"+
                                   "</div>"
                                  )
                           },
                         "title":"Actions", "orderable": false, "className":"text-center"
                       }
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableSortieAna').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.AO,
            rowId: "id",
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                       { "data": "libelle",    "title":"Libellé",    "className": "" },
                       { "data": "type",   "title":"Type",   "className": "hidden-xs" },
                       { "data": "min", "title":"min", "className": "hidden-xs" },
                       { "data": "max", "title":"max", "className": "hidden-xs" },
                       { "data": null,
                         "render": function (item)
                          { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                   "    <button class='btn btn-outline-primary btn-sm' "+
                                               "onclick=window.location.href='atelier/"+item.id+"' "+
                                               "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                               "<i class='fas fa-image'></i></button>"+
                                   "    <button class='btn btn-danger btn-sm' "+
                                               "onclick=Show_Modal_Del("+item.acronyme+") "+
                                               "data-toggle='tooltip' title='Supprimer le mnémonique'>"+
                                               "<i class='fas fa-trash'></i></button>"+
                                   "</div>"
                                  )
                           },
                         "title":"Actions", "orderable": false, "className":"text-center"
                       }
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableCI').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.CI,
            rowId: "id",
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center align-middle" },
                       { "data": "etat",       "title":"Etat", "className": "text-center align-middle" },
                       { "data": "valeur",     "title":"Valeur",     "className": "text-center align-middle" },
                       { "data": "multi",      "title":"Multiplicateur", "className": "text-center align-middle hidden-xs" },
                       { "data": "unite",      "title":"Unité", "className": "text-center align-middle hidden-xs" },
/*                       { "data": null,
                         "render": function (item)
                          { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                   "    <button class='btn btn-outline-primary btn-sm' "+
                                               "onclick=window.location.href='atelier/"+item.id+"' "+
                                               "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                               "<i class='fas fa-image'></i></button>"+
                                   "    <button class='btn btn-danger btn-sm' "+
                                               "onclick=Show_Modal_Del("+item.acronyme+") "+
                                               "data-toggle='tooltip' title='Supprimer le mnémonique'>"+
                                               "<i class='fas fa-trash'></i></button>"+
                                   "</div>"
                                  )
                           },
                         "title":"Actions", "orderable": false, "className":"text-center"
                       }*/
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableRegistre').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.REGISTRE,
            rowId: "id",
            columns:
              [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "libelle",    "title":"Libellé",    "className": "" },
                { "data": "valeur",     "title":"Valeur en base",   "className": "hidden-xs" },
                { "data": "unite",      "title":"Unité",    "className": "hidden-xs" },
                { "data": null, "className": "",
                  "title":"Archivage", "orderable": true,
                  "render": function (item)
                    { if (item.archivage==true)
                       { return( Bouton ( "success", "Archivage activé", null, null, "Actif" ) );
                       }
                      else
                       { return( Bouton ( "outline-secondary", "Archivage désactivé", null, null, "Inactif" ) );
                       }
                    },
                },
                { "data": "map_question_vocale",    "title":"Question Vocale",    "className": "hidden-xs" },
                { "data": "map_reponse_vocale",    "title":"Reponse Vocale",    "className": "hidden-xs" }
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableTempo').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.T,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "etat",       "title":"Etat",     "className": "" },
                { "data": "status",     "title":"Status",   "className": "hidden-xs" },
                { "data": "daa",        "title":"daa",      "className": "hidden-xs" },
                { "data": "dma",        "title":"dma",      "className": "hidden-xs" },
                { "data": "dMa",        "title":"dMa",      "className": "hidden-xs" },
                { "data": "dad",        "title":"dad",      "className": "hidden-xs" },
                { "data": "date_on",    "title":"date_on",  "className": "hidden-xs" },
                { "data": "date_off",   "title":"date_off", "className": "hidden-xs" },
/*                { "data": null, "className": "",
                  "title":"Archivage", "orderable": true,
                  "render": function (item)
                    { if (item.archivage==true)
                       { return( Bouton ( "success", "Archivage activé", null, null, "Actif" ) );
                       }
                      else
                       { return( Bouton ( "outline-secondary", "Archivage désactivé", null, null, "Inactif" ) );
                       }
                    },
                },*/
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableBool').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.BOOL,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "etat",       "title":"Etat",     "className": "" },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableVisuel').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.I,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "mode",       "title":"Mode",     "className": "" },
                { "data": "color",      "title":"Couleur",     "className": "" },
                { "data": "cligno",     "title":"Cligno",     "className": "" },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableMessages').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.MSG,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "etat",       "title":"Etat",     "className": "" },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

     };
    xhr.send();
    $('#idTabEntreeTor').tab('show');

  }
