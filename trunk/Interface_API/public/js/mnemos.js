 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log ("in load page !");
    $('#idTabEntreeTor').tab('show');
    $('#idTableEntreeTor').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list/"+vars[4],	type : "GET", dataSrc: "DI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
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

  }
