 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Motif_to_canvas ( canvas, Motif )
  {
    var request = new XMLHttpRequest();                         /* Envoie une requete de récupération du SVG associé au motif */
console.log("Load_motif_to_canvas "+canvas+" "+Motif );
    request.open("GET", "https://icons.abls-habitat.fr/assets/gif/"+Motif.forme+".svg", true);
    request.onreadystatechange = function()
     { if (request.readyState == 4 && (request.status == 200 || request.status == 0))
       { var svg = request.responseXML.documentElement;
         svg.motif = Motif;                                                            /* Sauvegarde du pointeur Motif source */
         if(Motif.icone==428) console.debug(Motif);
         svg.setAttribute( "id", "WTD-motif-"+svg.motif.id );
         svg.setAttribute( "class", "WTDControl_bit_" + Motif.bitctrl );                        /* Affectation du control bit */
         svg.currentColor  = Motif.def_color;
         svg.currentState  = 0;
         svg.currentCligno = 0;
         svg.ChangeState = function ( state, color, cligno )                                 /* Fonction de changement d'etat */
           { if (color==undefined) color=this.motif.def_color;
             targets = this.getElementsByClassName('WTD-ActiveFillColor');
             for (target of targets)
              { var chg_matrice = { fill: [ this.currentColor, color ] };                            /* Matrice de changement */
                var chg_timing  = { duration: 500, fill: 'forwards' };                                /* Timing de changement */
                target.animate ( chg_matrice, chg_timing );                                                   /* Go Animate ! */
              }
             targets = this.getElementsByClassName('WTD-ActiveStrokeColor');
             for (target of targets)
              { var chg_matrice = { stroke: [ this.currentColor, color ] };                          /* Matrice de changement */
                var chg_timing  = { duration: 500, fill: 'forwards' };                                /* Timing de changement */
                target.animate ( chg_matrice, chg_timing );                                                   /* Go Animate ! */
              }
             this.currentColor = color;
             this.ChangeCligno ( cligno );
           }
                                                                                       /* ajout des objects de clignottements */
         var myanim=document.createElementNS("http://www.w3.org/2000/svg", 'animate');
         myanim.setAttribute("id","fadeout");
         myanim.setAttribute("class","WTD-animate");
         myanim.setAttribute("attributeName","opacity");
         myanim.setAttribute("from","1");
         myanim.setAttribute("to","0.5");
         myanim.setAttribute("dur","0.7s");
         myanim.setAttribute("fill","freeze");
         myanim.setAttribute("begin","indefinite");
         myanim.addEventListener ( "end", function (event)
                                     { svg.getElementById("fadein").beginElement(); }, false );
         svg.appendChild(myanim);

         var myanim=document.createElementNS("http://www.w3.org/2000/svg", 'animate');
         myanim.setAttribute("id","fadein");
         myanim.setAttribute("class","WTD-animate");
         myanim.setAttribute("attributeName","opacity");
         myanim.setAttribute("from","0.5");
         myanim.setAttribute("to","1");
         myanim.setAttribute("dur","0.3s");
         myanim.setAttribute("fill","freeze");
         myanim.addEventListener ( "end", function (event)
                                     { if (svg.currentCligno==1) svg.getElementById("fadeout").beginElement(); }, false );
         svg.appendChild(myanim);

         svg.ChangeCligno = function ( cligno )
           { if (cligno==this.currentCligno) return;
             if (cligno==1)
              { svg.currentCligno=1;
                svg.getElementById("fadeout").beginElement();
              }
             else
              { svg.currentCligno=0;
              }
           }
                                                                                                  /* Connexion aux listeners  */
/*         svg.addEventListener ( "click", function (event) { Clic_sur_motif ( svg ) }, false);*/
         $("#"+canvas).append(svg);                                                           /* ajout du SVG dans le Top SVG */
         setTimeout(function(){ svg.ChangeState ( 0, Motif.def_color, false ); }, 500);
       }
     };
    request.send(null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");
test = 0;    vars = window.location.pathname.split('/');
    $('#idTableMotif').DataTable(
       { pageLength : 100,
         fixedHeader: true,
         ajax: {	url : "/api/syn/show/"+vars[3],	type : "GET", dataSrc: "motifs",
                 error: function ( xhr, status, error ) { Show_Error(xhr.statusText); }
               },
         rowId: "id",
         columns: [ { "data": "id", "title":"#", "className": "text-center " },
                    { "data": null, "title":"Visuel", "className": "text-center",
                      "render": function (item, type, row, meta)
                        { console.log("Datatable test="+test+" id="+item.id);
                          test=test+1;
                          /*Load_Motif_to_canvas( "WtdCanvas-"+item.id, item );*/
                          return("<div id='WtdCanvas-"+item.id+"' ></div>");

                        }
                    },
                    { "data": "libelle", "title":"Libellé" },
                    { "data": "access_level", "title":"Level" },
                    { "data": "tech_id", "title":"Tech_id" },
                    { "data": "acronyme", "title":"Acronyme" },
                    { "data": "clic_tech_id", "title":"Clic Tech_id" },
                    { "data": "clic_acronyme", "title":"Clic Acronyme" },
/*                    { "data": null,
                      "render": function (item)
                       { return("<div class='btn-group btn-block' role='group' aria-label='ButtonGroup'>"+
                                "    <button class='btn btn-outline-primary btn-sm' "+
                                            "onclick=window.location.href='atelier/"+item.id+"' "+
                                            "data-toggle='tooltip' title='Ouvrir Atelier'>"+
                                            "<i class='fas fa-image'></i></button>"+
                                "    <button class='btn btn-outline-secondary btn-sm' "+
                                            "onclick=Show_Modal_Edit("+item.id+") "+
                                            "data-toggle='tooltip' title='Configurer le synoptique'>"+
                                            "<i class='fas fa-pen'></i></button>"+
                                "    <button class='btn btn-outline-success btn-sm' "+
                                            "onclick=Show_Modal_Add('"+item.page+"') "+
                                            "data-toggle='tooltip' title='Ajoute un synoptique fils'>"+
                                            "<i class='fas fa-plus'></i></button>"+
                                "    <button class='btn btn-outline-info btn-sm' "+
                                            "data-toggle='tooltip' title='Ajout un module D.L.S'>"+
                                            "<i class='fas fa-plus'></i> <i class='fas fa-code'></i></button>"+
                                "    <button class='btn btn-danger btn-sm' "+
                                            "onclick=Show_Modal_Del("+item.id+") "+
                                            "data-toggle='tooltip' title='Supprimer le Synoptique\net ses dépendances'>"+
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
  }
