 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    var json_request = JSON.stringify(
     { search_for : vars[3],
     });

    Send_to_API ( "PUT", "/api/search", json_request, function(Response)
     {
       $('#idTableSearchResults').DataTable(
          { pageLength : 50,
            fixedHeader: true,
            rowId: "id",
            data: Response.results,
            columns:
             [ { "data": "classe", "title":"Classe", "className": "align-middle hidden-xs text-center" },
               { "data": "tech_id", "title":"tech_id", "className": "align-middle hidden-xs text-center" },
               { "data": "acronyme", "title":"acronyme", "className": "align-middle hidden-xs text-center" },
               { "data": "libelle", "title":"libelle", "className": "align-middle hidden-xs text-center" },
             ]
          }
       );
     }, null );
  }
