 document.addEventListener('DOMContentLoaded', Load_page, false);
 var Instances;

 function Mnemos_CI_disable_archivage ( tech_id )
  { var json_request = JSON.stringify(
       { classe   : "CI",
         tech_id  : tech_id,
         archivage: false
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { /*table = $('#idTableModbusMapAI').DataTable();
       selection = table.ajax.json().mappings.filter( function(item) { return (item.id==id) } )[0];
       /*$('#idTableModbus').DataTable().ajax.reload(null, false);*/
     });
  }

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
                       { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                       { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                       { "data": "map_tag",    "title":"map_tag",   "className": "hidden-xs" },
                       { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
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
                          }
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
                       { "data": null, "title":"Acronyme", "className": "text-center",
                         "render": function (item)
                          { return( Lien ( "/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR", "Voir la courbe", item.acronyme ) ); }
                       },
                       { "data": "libelle",    "title":"Libellé",    "className": "" },
                       { "data": "type",   "title":"Type",   "className": "hidden-xs" },
                       { "data": "min", "title":"min", "className": "hidden-xs" },
                       { "data": "max", "title":"max", "className": "hidden-xs" },
                       { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                       { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                       { "data": "map_tag",    "title":"map_tag",   "className": "hidden-xs" },
                       { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
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
                          }
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
                       { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                       { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                       { "data": "map_tag",    "title":"map_tag",    "className": "hidden-xs" },
                       { "data": "dst_param1", "title":"dst_param1", "className": "hidden-xs" },
                       { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
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
                          }
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
            columns: [ { "data": "tech_id",  "title":"TechId",     "className": "text-center hidden-xs" },
                       { "data": "acronyme", "title":"Acronyme",   "className": "text-center" },
                       { "data": "libelle",  "title":"Libellé",    "className": "" },
                       { "data": "type",     "title":"Type",   "className": "hidden-xs" },
                       { "data": "min",      "title":"min", "className": "hidden-xs" },
                       { "data": "max",      "title":"max", "className": "hidden-xs" },
                       { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                       { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                       { "data": "map_tag",    "title":"map_tag",    "className": "hidden-xs" },
                       { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
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
                          }
                       }
                     ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableRegistre').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            data: Response.REGISTRE,
            rowId: "id",
            columns:
              [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "libelle",    "title":"Libellé",    "className": "" },
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

       $('#idTableCptImp').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            data: Response.CI,
            rowId: "id",
            columns:
              [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "libelle",    "title":"Libellé",    "className": "hidden-xs" },
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Activé", null, null, "Actif" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Désactivé", null, null, "Inactif" ) ); }
                    },
                },
                { "data": "multi",      "title":"Multi.",   "className": "text-center hidden-xs" },
                { "data": "unite",      "title":"Unité",    "className": "text-center hidden-xs" },
                { "data": null, "title":"Archivage", "className": "hidden-xs",
                  "render": function (item)
                    { if (item.archivage==true)
                       { return( Bouton ( "success", "Cliquez pour désactiver", "Mnemos_CI_disable_archivage", item.tech_id, "Actif" ) );
                       }
                      else
                       { return( Bouton ( "outline-secondary", "Cliquez pour activer", "Mnemos_CI_enable_archivage", item.tech_id, "Inactif" ) );
                       }
                    }
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
