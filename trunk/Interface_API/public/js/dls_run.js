 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Dls_start_plugin ( tech_id )
  { Send_to_API ( 'POST', "/api/dls/start/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

 function Dls_stop_plugin ( tech_id )
  { Send_to_API ( 'POST', "/api/dls/stop/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/dls/run", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       $('#idTableRunDLS').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false,
            data: Response.plugins,
            rowId: "id",
            columns:
             [ { "data": "tech_id",    "title":"TechId",     "className": "text-center align-middle" },
               { "data": "version",    "title":"Version",   "className": "text-center align-middle hidden-xs" },
               { "data": null, title:"Started",  "className": "text-center align-middle",
                 "render": function (item)
                 { if (item.started==true)
                   { return( Bouton ( "success", "Désactiver le plugin",
                                      "Dls_stop_plugin", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le plugin",
                                      "Dls_start_plugin", item.tech_id, "Désactivé" ) );
                   }
                 }
               },
               { "data": "start_date", "title":"Start Date",   "className": "text-center align-middle hidden-xs" },
               { "data": null, title:"Conso",  "className": "text-center align-middle", "render": function (item)
                  { return( item.conso.toFixed(2) ); }
               },
               { "data": null, title:"Debug",  "className": "text-center align-middle", "render": function (item)
                 { if (item.debug==true)
                   { return( Bouton ( "warning", "Désactiver le debug",
                                      "Dls_debug_plugin", item.tech_id, "Actif" ) );
                   }
                  else
                   { return( Bouton ( "outline-secondary", "Activer le débug",
                                      "Dls_undebug_plugin", item.tech_id, "Désactivé" ) );
                   }
                 }
               },
               { "data": null, title:"Comm",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_comm_out==true)
                   { return( Bouton ( "danger", "Communication perdue", null, null, "Hors Comm" ) ); }
                  else
                   { return( Bouton ( "outline-success", "Communication OK", null, null, "OK" ) ); }
                 }
               },
               { "data": null, title:"Défaut",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_defaut==false)
                   { return( Bouton ( "outline-secondary", "Pas de défaut", null, null, "Non" ) ); }
                  else if (item.bit_defaut_fixe==true)
                   { return( Bouton ( "warning", "Défaut Fixe", null, null, "Fixe" ) ); }
                  else { return( Bouton ( "danger", "Défaut !", null, null, "OUI" ) ); }
                 }
               },
               { "data": null, title:"Alarme",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_alarme==false)
                   { return( Bouton ( "outline-secondary", "Pas d'alarme", null, null, "Non" ) ); }
                  else if (item.bit_alarme_fixe==true)
                   { return( Bouton ( "warning", "Alarme Fixe", null, null, "Fixe" ) ); }
                  else { return( Bouton ( "danger", "Alarme !", null, null, "OUI" ) ); }
                 }
               },
               { "data": null, title:"Alerte",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_alerte==false)
                   { return( Bouton ( "outline-secondary", "Pas d'alerte", null, null, "Non" ) ); }
                  else if (item.bit_alerte_fixe==true)
                   { return( Bouton ( "warning", "Alerte Fixe", null, null, "Fixe" ) ); }
                  else { return( Bouton ( "danger", "Alerte !", null, null, "OUI" ) ); }
                 }
               },
               { "data": null, title:"Veille",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_veille==true)
                   { return( Bouton ( "outline-success", "En veille", null, null, "OK" ) ); }
                  else { return( Bouton ( "danger", "Pas en veille !", null, null, "NON" ) ); }
                 }
               },
               { "data": null, title:"Danger",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_danger==false)
                   { return( Bouton ( "outline-secondary", "Pas de danger", null, null, "Non" ) ); }
                  else if (item.bit_danger_fixe==true)
                   { return( Bouton ( "warning", "Danger Fixe", null, null, "Fixe" ) ); }
                  else { return( Bouton ( "danger", "Danger !", null, null, "OUI" ) ); }
                 }
               },
               { "data": null, title:"Dérang.",  "className": "text-center align-middle", "render": function (item)
                 { if (item.bit_derangement==false)
                   { return( Bouton ( "outline-secondary", "Pas de dérangement", null, null, "Non" ) ); }
                  else if (item.bit_derangement_fixe==true)
                   { return( Bouton ( "warning", "Dérangement Fixe", null, null, "Fixe" ) ); }
                  else { return( Bouton ( "danger", "Dérangement !", null, null, "OUI" ) ); }
                 }
               },
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
            fixedHeader: true, paging: false, ordering: false, searching: false,
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
  }
