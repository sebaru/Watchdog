 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    var json_request = JSON.stringify(
     { search_for : vars[3],
     });

    Send_to_API ( "PUT", "/api/search", json_request, function(Response)
     { $("#idSearch").val ( vars[3] );
       $('#idTableSearchResults').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            rowId: "id",
            data: Response.results,
            columns:
             [ { "data": "classe", "title":"Classe", "className": "align-middle  text-center" },
               { "data": null, "title":"TechID/Page", "className": "align-middle ",
                 "render": function (item)
                   { if (item.classe=="SYNOPTIQUE")
                      { return( Lien ( "/home/syn/"+item.tech_id, "Voir le synoptique", item.tech_id ) ); }
                     else return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) );
                   }
               },
               { "data": null, "title":"Acronyme", "className": "align-middle ",
                 "render": function (item)
                   { if (item.classe=="MESSAGE")
                      { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.tech_id ) ); }
                     else if (item.classe=="CI")
                      { return( Lien ( "/home/archive/"+item.tech_id+"/"+item.acronyme, "Voir le graphe", item.acronyme ) ); }
                     else if (item.classe=="AI")
                      { return( Lien ( "/home/archive/"+item.tech_id+"/"+item.acronyme, "Voir le graphe", item.acronyme ) ); }
                     else return(item.acronyme);
                   }
               },
               { "data": "libelle", "title":"libelle", "className": "align-middle  text-center" },
             ]
          }
       );
     }, null );
  }
