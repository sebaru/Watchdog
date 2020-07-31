 document.addEventListener('DOMContentLoaded', Load_page, false);
 var Instances;

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log ("in load page !");
    $('#idTitle').html(vars[3]);

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/instance/list", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       Instance = Response.instance;
     };
    xhr.send();


    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/mnemos/list/"+vars[3], true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       $('#idTableEntreeTor').DataTable(
          { pageLength : 50,
            fixedHeader: true,
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

       $('#idTableEntreeAna').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            data: Response.AI,
               rowId: "id",
            columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
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

       $('#idTableSortieTor').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            data: Response.DO,
            rowId: "id",
            columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
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
            fixedHeader: true,
            data: Response.AO,
            rowId: "id",
            columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
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

       $('#idTableRegistre').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            data: Response.DO,
            rowId: "id",
            columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
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


     };
    xhr.send();
    $('#idTabEntreeTor').tab('show');

  }
