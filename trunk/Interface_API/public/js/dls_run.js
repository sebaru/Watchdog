 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Dls_start_plugin ( tech_id )
  { Send_to_API ( 'PUT', "/api/dls/start/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

 function Dls_stop_plugin ( tech_id )
  { Send_to_API ( 'PUT', "/api/dls/stop/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

 function Dls_debug_plugin ( tech_id )
  { Send_to_API ( 'PUT', "/api/dls/debug/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

 function Dls_undebug_plugin ( tech_id )
  { Send_to_API ( 'PUT', "/api/dls/undebug/"+tech_id, function () { $('#idTableRunDLS').DataTable().ajax.reload(); } ); }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");

    $('#idTableRunDLS').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false,
       ajax: {	url : "/api/dls/run",	type : "GET", dataSrc: "plugins",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       data: Response.plugins,
       columns:
        [ { "data": null, "title":"tech_id", "className": "align-middle hidden-xs text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, item.version + "\n" + item.start_date, item.tech_id ) );
                },
            },
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
          { "data": null, title:"Conso",  "className": "text-center align-middle", "render": function (item)
             { return( item.conso.toFixed(2) ); }
          },
          { "data": null, title:"Debug",  "className": "text-center align-middle", "render": function (item)
            { if (item.debug==true)
               { return( Bouton ( "warning", "Désactiver le debug",
                                  "Dls_undebug_plugin", item.tech_id, "Actif" ) );
               }
              else
               { return( Bouton ( "outline-secondary", "Activer le débug",
                                  "Dls_debug_plugin", item.tech_id, "Désactivé" ) );
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
              else { return( Bouton ( "danger", "Pas en veille !", null, null, "Non" ) ); }
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
     });
  }
