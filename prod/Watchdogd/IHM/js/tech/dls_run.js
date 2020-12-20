 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Go_to_dls_source ()
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
    var json_request = JSON.stringify(
     { tech_id : vars[3],
     });
    Send_to_API ( "PUT", "/api/dls/run", json_request, function(Response)
     { $('#idTableEntreeTOR').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.DI,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
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
              [ { "data": null, "title":"Acronyme", "className": "align-middle text-center",
                  "render": function (item)
                    { return ( Lien ("/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR'", "Voir le graphe", item.acronyme ) ); },
                },
                { "data": "valeur", "title":"Valeur", "className": "align-middle text-center " },
                { "data": "unite", "title":"Unité", "className": "align-middle text-center " },
                { "data": null, "title":"in_range", "className": "align-middle",
                  "render": function (item)
                    { if (item.in_range==true) { return( Bouton ( "outline-success", "Dans les clous !", null, null, "Oui" ) );        }
                                          else { return( Bouton ( "warning", "Pb !", null, null, "Non" ) ); }
                    },
                },
                { "data": "last_arch", "title":"last_arch", "className": "align-middle text-center " },
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
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
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
            columns: [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                       { "data": "libelle",    "title":"Libellé",    "className": "align-middle " },
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
              [ { "data": null, "title":"Acronyme", "className": "align-middle text-center",
                  "render": function (item)
                    { return ( Lien ("/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR'", "Voir le graphe", item.acronyme ) ); },
                },
                { "data": null, "title":"Etat", "className": "",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "Le bit est a 1", null, null, "1" ) );        }
                                      else { return( Bouton ( "outline-secondary", "Le bit est a 0", null, null, "0" ) ); }
                    },
                },
                { "data": "valeur",     "title":"Valeur",     "className": "text-center align-middle" },
                { "data": "multi",      "title":"Multiplicateur", "className": "text-center align-middle " },
                { "data": "unite",      "title":"Unité", "className": "text-center align-middle " },
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
              [ { "data": null, "title":"Acronyme", "className": "align-middle text-center",
                  "render": function (item)
                    { return ( Lien ("/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR'", "Voir le graphe", item.acronyme ) ); },
                },
                { "data": "valeur",     "title":"Valeur",   "className": "align-middle " },
                { "data": "unite",      "title":"Unité",    "className": "align-middle " },
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
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
                  "render": function (item)
                    { if (item.etat==true) { return( Bouton ( "success", "La tempo compte le temps", null, null, "En décompte" ) );        }
                                      else { return( Bouton ( "outline-secondary", "La tempo ne compte pas", null, null, "Stoppée" ) ); }
                    },
                },
                { "data": "status",     "title":"Status",   "className": "align-middle  text-center" },
                { "data": "daa",        "title":"daa",      "className": "align-middle  text-center" },
                { "data": "dma",        "title":"dma",      "className": "align-middle  text-center" },
                { "data": "dMa",        "title":"dMa",      "className": "align-middle  text-center" },
                { "data": "dad",        "title":"dad",      "className": "align-middle  text-center" },
                { "data": "date_on",    "title":"date_on",  "className": "align-middle  text-center" },
                { "data": "date_off",   "title":"date_off", "className": "align-middle  text-center" },
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
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
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
            data: Response.VISUEL,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": "mode",       "title":"Mode",     "className": "align-middle text-center" },
                { "data": "color",      "title":"Couleur",     "className": "align-middle text-center" },
                { "data": null, "title":"Cligno", "className": "align-middle ",
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

       $('#idTableWatchdog').DataTable(
          { pageLength : 50,
            fixedHeader: true, paging: false, ordering: false, searching: false,
            data: Response.WATCHDOG,
            rowId: "id",
            columns:
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
                  "render": function (item)
                    { if (item.etat==false) { return( Bouton ( "success", "Le compteur décompte", null, null, "En décompte" ) );  }
                                       else { return( Bouton ( "outline-warning", "Le compteur est échu", null, null, "échu" ) ); }
                    },
                },
                { "data": "decompte",   "title":"Reste en décompte", "className": "align-middle text-center" },
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
              [ { "data": "acronyme",   "title":"Acronyme",   "className": "align-middle text-center" },
                { "data": null, "title":"Etat", "className": "align-middle ",
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

     }, null);

    $('#idTabEntreeTor').tab('show');

  }
