 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { Send_to_API ( "GET", "/api/search", null, function(Response)
     { $('#idTableSearchResults').DataTable(
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
               { "data": "libelle", "title":"Libelle", "className": "align-middle  text-center" },
               { "data": "unite", "title":"Unité", "className": "align-middle  text-center" },
             ]
          }
       );
     }, null );
  }
