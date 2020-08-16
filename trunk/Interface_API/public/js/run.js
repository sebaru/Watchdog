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
       $('#idTableEntreeTOR').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.DI,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Activée", null, null, "Active" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Désactivée", null, null, "Inactive" ) ); }
                    },
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
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "valeur", "title":"Valeur", "className": "text-center hidden-xs" },
                { "data": "unite", "title":"Unité", "className": "text-center hidden-xs" },
                { "data": null, "title":"in_range", "className": "",
                  "render": function (item)
                    { if (item.in_range==true) { return( Bouton ( "outline-success", "Dans les clous !", null, null, "Oui" ) );        }
                                          else { return( Bouton ( "warning", "Pb !", null, null, "Non" ) ); }
                    },
                },
                { "data": "last_arch", "title":"last_arch", "className": "text-center hidden-xs" },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableSortieTOR').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.DO,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Activée", null, null, "Active" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Désactivée", null, null, "Inactive" ) ); }
                    },
                },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

       $('#idTableSortieANA').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.AO,
            rowId: "id",
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                       { "data": "libelle",    "title":"Libellé",    "className": "" },
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
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center align-middle" },
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
                    },
                },
                { "data": "valeur",     "title":"Valeur",     "className": "text-center align-middle" },
                { "data": "multi",      "title":"Multiplicateur", "className": "text-center align-middle hidden-xs" },
                { "data": "unite",      "title":"Unité", "className": "text-center align-middle hidden-xs" },
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
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                { "data": "valeur",     "title":"Valeur",   "className": "hidden-xs" },
                { "data": "unite",      "title":"Unité",    "className": "hidden-xs" },
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
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "La tempo compte le temps", null, null, "En décompte" ) );        }
                                      else { return( Bouton ( "outline-secondary", "La tempo ne compte pas", null, null, "Stoppée" ) ); }
                    },
                },
                { "data": "status",     "title":"Status",   "className": "hidden-xs text-center" },
                { "data": "daa",        "title":"daa",      "className": "hidden-xs text-center" },
                { "data": "dma",        "title":"dma",      "className": "hidden-xs text-center" },
                { "data": "dMa",        "title":"dMa",      "className": "hidden-xs text-center" },
                { "data": "dad",        "title":"dad",      "className": "hidden-xs text-center" },
                { "data": "date_on",    "title":"date_on",  "className": "hidden-xs text-center" },
                { "data": "date_off",   "title":"date_off", "className": "hidden-xs text-center" },
              ],
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
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
                    },
                },
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
                { "data": "mode",       "title":"Mode",     "className": "text-center" },
                { "data": "color",      "title":"Couleur",     "className": "text-center" },
                { "data": null, "title":"Cligno", "className": "",
                  "render": function (item)
                    { if (item.cligno==true) { return( Bouton ( "outline-success", "Le visuel clignote", null, null, "Oui" ) );          }
                                        else { return( Bouton ( "outline-secondary", "Le visuel ne clignote pas", null, null, "Non" ) ); }
                    },
                },
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
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Le message est a 1", null, null, "Actif" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Le message est a 0", null, null, "Inactif" ) ); }
                    },
                },
              ],
            /*order: [ [0, "desc"] ],*/
            responsive: true,
          }
        );

     };
    xhr.send();
    $('#idTabEntreeTor').tab('show');

  }
