 document.addEventListener('DOMContentLoaded', Load_page, false);

 function Dls_start_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/start", json_request, function ()
     { $('#idTableRunDLS').DataTable().ajax.reload();
     });
  }

 function Dls_stop_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/stop", json_request, function ()
     { $('#idTableRunDLS').DataTable().ajax.reload();
     });
  }

 function Dls_acquitter_plugin ( tech_id )
  { var json_request = JSON.stringify( { tech_id : tech_id } );
    Send_to_API ( 'POST', "/api/dls/acquitter", json_request, function ()
     { $('#idTableRunDLS').DataTable().ajax.reload();
     });
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");

    $('#idTableRunDLS').DataTable(
     { pageLength : 50,
       fixedHeader: true, paging: false,
       ajax: {	url : "/api/dls/status",	type : "GET", dataSrc: "plugins",
               error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
             },
       rowId: "id",
       data: Response.plugins,
       columns:
        [ { "data": null, "title":"TechID", "className": "align-middle  text-center",
              "render": function (item)
                { return( Lien ( "/tech/dls_source/"+item.tech_id, item.version + "\n" + item.start_date, item.tech_id ) );
                },
            },
          { "data": null, title:"Started",  "className": "text-center align-middle",
            "render": function (item)
             { if (item.started==true)
                { return( Bouton ( "success", "Désactiver le plugin", "Dls_stop_plugin", item.tech_id, "Actif" ) ); }
               else
                { return( Bouton ( "outline-secondary", "Activer le plugin", "Dls_start_plugin", item.tech_id, "Désactivé" ) ); }
             }
          },
          { "data": null, title:"Conso",  "className": "text-center align-middle", "render": function (item)
             { return( item.conso.toFixed(2) ); }
          },
          { "data": null, title:"Comm",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_comm==true)
               { return( Bouton ( "outline-success", "Communication OK", null, null, "OK" ) ); }
              else
               { return( Bouton ( "danger", "Communication perdue", null, null, "Hors Comm" ) ); }
            }
          },
          { "data": null, title:"Défaut",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_defaut_fixe==true)
               { return( Bouton ( "danger", "Défaut !", null, null, "OUI" ) ); }
              else if (item.bit_defaut==true)
               { return( Bouton ( "warning", "Défaut Fixe", null, null, "Fixe" ) ); }
              else
               { return( Bouton ( "outline-secondary", "Pas de défaut", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Alarme",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_alarme==true)
               { return( Bouton ( "danger", "Alarme !", null, null, "OUI" ) ); }
              else if (item.bit_alarme_fixe==true)
               { return( Bouton ( "warning", "Alarme Fixe", null, null, "Fixe" ) ); }
              else
               { return( Bouton ( "outline-secondary", "Pas d'alarme", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Alerte",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_alerte==true)
               { return( Bouton ( "danger", "Alerte !", null, null, "OUI" ) ); }
              else if (item.bit_alerte_fugitive==true)
               { return( Bouton ( "danger", "Alerte Fugitive", null, null, "Fugitive" ) ); }
              else if (item.bit_alerte_fixe==true)
               { return( Bouton ( "warning", "Alerte Fixe", null, null, "Fixe" ) ); }
              else
               { return( Bouton ( "outline-secondary", "Pas d'alerte", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Veille",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_veille==true)
               { return( Bouton ( "outline-success", "En veille", null, null, "OK" ) ); }
              else { return( Bouton ( "warning", "Pas en veille !", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Danger",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_danger==true)
               { return( Bouton ( "danger", "Danger !", null, null, "OUI" ) ); }
              else if (item.bit_danger_fixe==true)
               { return( Bouton ( "warning", "Danger Fixe", null, null, "Fixe" ) ); }
              else
               { return( Bouton ( "outline-secondary", "Pas de danger", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Dérang.",  "className": "text-center align-middle", "render": function (item)
            { if (item.bit_derangement==true)
               { return( Bouton ( "danger", "Dérangement !", null, null, "OUI" ) ); }
              else if (item.bit_derangement_fixe==true)
               { return( Bouton ( "warning", "Dérangement Fixe", null, null, "Fixe" ) ); }
              else
               { return( Bouton ( "outline-secondary", "Pas de dérangement", null, null, "Non" ) ); }
            }
          },
          { "data": null, title:"Acquitter",  "className": "text-center align-middle", "render": function (item)
            { return( Bouton ( "outline-info", "Acquitter le module", "Dls_acquitter_plugin", item.tech_id, "Acquitter" ) ); }
          },
        ],
      /*order: [ [0, "desc"] ],*/
       responsive: true,
     });
  }
