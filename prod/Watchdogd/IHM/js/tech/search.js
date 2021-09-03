 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { $('#idTableSearchResults').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         rowId: "id",
         processing: true,
         serverSide: true,
         ajax: {	url : "/api/search",	type : "GET",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
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
                   { return( Lien ( "/tech/dls_source/"+item.tech_id, "Voir la source", item.acronyme ) ); }
                  else if (item.classe=="CI")
                   { return( Lien ( "/tech/courbe/"+item.tech_id+"/"+item.acronyme, "Voir le graphe", item.acronyme ) ); }
                  else if (item.classe=="AI")
                   { return( Lien ( "/tech/courbe/"+item.tech_id+"/"+item.acronyme, "Voir le graphe", item.acronyme ) ); }
                  else if (item.classe=="HORLOGE")
                   { return( Lien ( "/horloge/"+item.id, "Editer les ticks", item.acronyme ) ); }
                  else return(item.acronyme);
                }
            },
            { "data": null, "title":"Libelle", "className": "align-middle  text-center",
              "render": function (item) { return ( htmlEncode ( item.libelle ) ); }
            },
            { "data": null, "title":"Unité", "className": "align-middle  text-center",
              "render": function (item) { return ( htmlEncode ( item.unite ) ); }
            }
          ]
       }
    );
  }
