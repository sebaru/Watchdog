 document.addEventListener('DOMContentLoaded', Load_page, false);
 var Instances;


 function Go_to_dls_source ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_source/"+vars[3] );
  }

 function Go_to_dls_run ()
  { vars = window.location.pathname.split('/');
    Redirect ( "/tech/dls_run/"+vars[3] );
  }

 function Mnemos_CI_enable_archivage ( acronyme )
  { table = $('#idTableCptImp').DataTable();
    selection = table.ajax.json().CI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "CI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: true
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableCptImp').DataTable().ajax.reload(null, false);
     });
  }

 function Mnemos_CI_disable_archivage ( acronyme )
  { table = $('#idTableCptImp').DataTable();
    selection = table.ajax.json().CI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "CI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         archivage: false
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, function ()
     { $('#idTableCptImp').DataTable().ajax.reload(null, false);
     });
  }
/******************************************************************************************************************************/
 function Mnemos_DI_set ( acronyme )
  { table = $('#idTableEntreeTor').DataTable();
    selection = table.ajax.json().DI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "DI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         etat     : true
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, null, null );
  }
/******************************************************************************************************************************/
 function Mnemos_DI_reset ( acronyme )
  { table = $('#idTableEntreeTor').DataTable();
    selection = table.ajax.json().DI.filter( function(item) { return (item.acronyme==acronyme) } )[0];
    var json_request = JSON.stringify(
       { classe   : "DI",
         tech_id  : selection.tech_id,
         acronyme : selection.acronyme,
         etat     : false
       }
     );

    Send_to_API ( 'POST', "/api/mnemos/set", json_request, null, null );
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log ("in load page !");
    $('#idTitle').html(vars[3]);
    var tech_id = vars[3];

    $('#idTableEntreeTor').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "DI", "tech_id": tech_id }, dataSrc: "DI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                    { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                    { "data": "libelle",    "title":"Libellé",    "className": "" },
                    { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                    { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                    { "data": "map_tag",    "title":"map_tag",   "className": "hidden-xs" },
                    { "data": null, "title":"Actions", "orderable": false, "className":"text-center",
                      "render": function (item)
                        { boutons = Bouton_actions_start ();
                          boutons += Bouton_actions_add ( "outline-success", "Active cette entrée", "Mnemos_DI_set", item.acronyme, "power-off", null );
                          boutons += Bouton_actions_add ( "outline-secondary", "Désactive cette entrée", "Mnemos_DI_reset", item.acronyme, "power-off", null );
                          boutons += Bouton_actions_end ();
                          return(boutons);
                        },
                    }
                  ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableEntreeAna').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "AI", "tech_id": tech_id }, dataSrc: "AI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                    { "data": null, "title":"Acronyme", "className": "text-center",
                      "render": function (item)
                       { return( Lien ( "/home/archive/"+item.tech_id+"/"+item.acronyme+"/HOUR", "Voir la courbe", item.acronyme ) ); }
                    },
                    { "data": "libelle",    "title":"Libellé",    "className": "" },
                    { "data": "unite",   "title":"Unité",   "className": "hidden-xs" },
                    { "data": "type",   "title":"Type",   "className": "hidden-xs" },
                    { "data": "min", "title":"min", "className": "hidden-xs" },
                    { "data": "max", "title":"max", "className": "hidden-xs" },
                    { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                    { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                    { "data": "map_tag",    "title":"map_tag",   "className": "hidden-xs" },
                  ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableSortieTor').DataTable(
       { pageLength : 50,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "DO", "tech_id": tech_id }, dataSrc: "DO",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },

         rowId: "id",
         columns: [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
                    { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
                    { "data": "libelle",    "title":"Libellé",    "className": "" },
                    { "data": "map_thread", "title":"map_thread", "className": "hidden-xs" },
                    { "data": "map_tech_id","title":"map_tech_id","className": "hidden-xs" },
                    { "data": "map_tag",    "title":"map_tag",    "className": "hidden-xs" },
                    { "data": "dst_param1", "title":"dst_param1", "className": "hidden-xs" },
                  ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableSortieAna').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "AO", "tech_id": tech_id }, dataSrc: "AO",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
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
                  ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableRegistre').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "R", "tech_id": tech_id }, dataSrc: "R",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
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
                    { return( Bouton ( "success", "Archivage activé", "Mnemos_set_R", item.tech_id, "Actif" ) );
                    }
                   else
                    { return( Bouton ( "outline-secondary", "Archivage désactivé", "Mnemos_set_R", item.tech_id, "Inactif" ) );
                    }
                 },
             },
             { "data": "map_question_vocale",   "title":"Question Vocale",   "className": "hidden-xs" },
             { "data": "map_reponse_vocale",    "title":"Reponse Vocale",    "className": "hidden-xs" },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableCptImp').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "CI", "tech_id": tech_id }, dataSrc: "CI",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
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
                    { return( Bouton ( "success", "Cliquez pour désactiver", "Mnemos_CI_disable_archivage", item.acronyme, "Actif" ) );
                    }
                   else
                    { return( Bouton ( "outline-secondary", "Cliquez pour activer", "Mnemos_CI_enable_archivage", item.acronyme, "Inactif" ) );
                    }
                 }
             },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTableMessage').DataTable(
       { pageLength : 50,
         fixedHeader: true,
         ajax: {	url : "/api/mnemos/list",	type : "GET", data: { "classe": "MSG", "tech_id": tech_id }, dataSrc: "MSG",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns:
           [ { "data": "tech_id",    "title":"TechId",     "className": "text-center hidden-xs" },
             { "data": "acronyme",   "title":"Acronyme",   "className": "text-center" },
             { "data": "libelle",    "title":"Libellé",    "className": "hidden-xs" },
             { "data": "audio", "title":"Audio ?",    "className": "hidden-xs" },
             { "data": "libelle_audio", "title":"Audio",    "className": "hidden-xs" },
             { "data": "sms", "title":"SMS ?",    "className": "hidden-xs" },
             { "data": "libelle_sms", "title":"SMS",    "className": "hidden-xs" },
             { "data": "profil_audio", "title":"Profil Audio",    "className": "hidden-xs" },
             { "data": "enable", "title":"Enable",    "className": "hidden-xs" },
             { "data": "etat", "title":"Etat",    "className": "hidden-xs" },
           ],
         /*order: [ [0, "desc"] ],*/
         responsive: true,
       }
     );

    $('#idTabEntreeTor').tab('show');

  }
