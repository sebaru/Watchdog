 document.addEventListener('DOMContentLoaded', Load_log, false);

 function Show_Modal_Edit ( syn_id )
  { /*$('#idModalSynDetail').innerHtml = "Vos identifiants et mots de passe sont incorrects";*/
    $('#idModalSynEdit').modal("show");
    console.debug(syn_id);
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_log ()
  { console.log ("in load synoptique !");
    $('#idTableSyn').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/syn/list",	type : "GET", dataSrc: "synoptiques" },
         columns: [ { "data": "id", "title":"#", "className": "text-center hidden-xs" },
                    { "data": "access_level", "title": "Level", "className": "hidden-xs" },
                    { "data": "ppage", "title": "Parent", "className": "hidden-xs" },
                    { "data": "page", "title":"Page" },
                    { "data": "libelle", "title":"Description" },
                    { "data": null,
                      "render": function (item)
                       { return("<div class='btn-group' role='group' aria-label='ButtonGroup'>"+
                                "  <button id='btnGroupAction' type='button' class='btn btn-secondary dropdown-toggle' data-toggle='dropdown' aria-haspopup='true' aria-expanded='false'>"+
                                "    Actions</button>"+
                                "  <div class='dropdown-menu dropdown-menu-right' aria-labelledby='btnGroupAction'>"+
                                "    <a class='dropdown-item'><i class='fas fa-image text-primary'></i> Ouvrir l'atelier</a>"+
                                "    <a class='dropdown-item' onclick='Show_Modal_Edit("+item.id+")'><i class='fas fa-wrench text-primary'></i> Configurer</a>"+
                                "    <a class='dropdown-item'><i class='fas fa-image text-danger'></i> Ajouter un Fils</a>"+
                                "    <a class='dropdown-item'><i class='fas fa-code text-info'></i> Ajouter un D.L.S</a>"+
                                "    <div class='dropdown-divider'></div>"+
                                "    <a class='dropdown-item  bg-danger text-white'><i class='fas fa-trash'></i> Supprimer</a>"+
                                "  </div>"+
                                "</div>"
                               )
                        },
                      "title":"Actions", "orderable": false, "className":"text-right"
                    }
                  ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
