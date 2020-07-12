 document.addEventListener('DOMContentLoaded', Load_syn, false);

 function Show_Modal_Edit ( syn_id )
  { table = $('#idTableSyn').DataTable();
    console.debug ( table.cell( "#"+syn_id, "libelle:name" ).data() );
    /*$('#idModalSynEditDescription').html ( table.cell( "#"+syn_id, "libelle:name" ).data() );*/
    $('#idModalSynEditDescription').val(table.cell( "#"+syn_id, "libelle:name" ).data());
    $('#idModalSynEdit').modal("show");
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_syn ()
  { console.log ("in load synoptique !");
    $('#idTableSyn').DataTable(
       { pageLength : 25,
         fixedHeader: true,
         ajax: {	url : "/api/syn/list",	type : "GET", dataSrc: "synoptiques" },
         rowId: "id",
         columns: [ { "data": "id", "title":"#", "className": "text-center hidden-xs" },
                    { "data": "access_level", name: "access_level", "title": "Level", "className": "hidden-xs" },
                    { "data": "ppage", "title": "Parent", "className": "hidden-xs" },
                    { "data": "page", "title":"Page" },
                    { "data": "libelle", "name": "libelle", "title":"Description" },
                    { "data": null,
                      "render": function (item)
                       { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                "    <button class='btn btn-outline-primary btn-sm' "+
                                            "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                            "<i class='fas fa-image'></i></button>"+
                                "    <button class='btn btn-outline-secondary btn-sm' "+
                                            "onclick=Show_Modal_Edit("+item.id+") "+
                                            "data-toggle='tooltip' title='Configurer le synoptique'>"+
                                            "<i class='fas fa-pen'></i></button>"+
                                "    <button class='btn btn-outline-success btn-sm' "+
                                            "data-toggle='tooltip' title='Ajoute un synoptique fils'>"+
                                            "<i class='fas fa-plus'></i></button>"+
                                "    <button class='btn btn-outline-info btn-sm' "+
                                            "data-toggle='tooltip' title='Ajout un module D.L.S'>"+
                                            "<i class='fas fa-plus'></i> <i class='fas fa-code'></i></button>"+
                                "    <button class='btn btn-danger btn-sm' "+
                                            "data-toggle='tooltip' title='Supprimer le Synoptique\net ses dépendances'>"+
                                            "<i class='fas fa-trash'></i></button>"+
                                "</div>"
                               )
                        },
                      "title":"Actions", "orderable": false, "className":"text-center"
                    }
                  ],
         order: [ [0, "desc"] ],
         responsive: true,
       }
     );
  }
