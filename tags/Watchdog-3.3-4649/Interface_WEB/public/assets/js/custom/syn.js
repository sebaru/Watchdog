	$(document).ready(function() {

	});


 var svg_selected;
 var syn_id;

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_syn ( id )
  { var topsvg = document.getElementById("TopSVG");
    if (!topsvg) return;
    syn_id = id;
    while (topsvg.firstChild) { topsvg.removeChild(topsvg.firstChild); }
    var listpass = document.getElementById("liste_passerelles");
    while (listpass.firstChild) { listpass.removeChild(listpass.firstChild); }
    var xhr = new XMLHttpRequest;
    xhr.open('get',base_url + "syn/get/" + id, true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */

       console.log("Traite Motif: "+Response.motifs.length);
       for (var i = 0; i < Response.motifs.length; i++)                          /* Pour chacun des motifs, parsing un par un */
        { var motif = Response.motifs[i];
          Load_Motif_to_canvas ( motif );
        }

       console.log("Traite Lien: "+Response.liens.length);
       for (var i = 0; i < Response.liens.length; i++)                            /* Pour chacun des liens, parsing un par un */
        { var lien = Response.liens[i];
          Load_Lien_to_canvas(lien);
        }


       console.log("Traite Rectangle: "+Response.rectangles.length);
       for (var i = 0; i < Response.rectangles.length; i++)                            /* Pour chacun des liens, parsing un par un */
        { var rectangle = Response.rectangles[i];
          Load_Rectangle_to_canvas(rectangle);
        }

       var listpass = document.getElementById("liste_passerelles");                                 /* Pour chaque passerelle */
       var button = document.createElement('button');
       button.setAttribute( "class", "btn btn-primary" );
       button.innerHTML = "Accueil";
       button.onclick = function() { Charger_syn(1); }
       listpass.appendChild(button);
       for (var i = 0; i < Response.passerelles.length; i++)                       /* Pour chacun des motifs, parsing un par un */
        { var passerelle = Response.passerelles[i];
          var button = document.createElement('button');
          button.setAttribute( "class", "btn btn-secondary" );
          button.innerHTML = passerelle.page;
          button.passerelle = passerelle;
          button.onclick = function() { Charger_syn(this.passerelle.syn_cible_id); };
          listpass.appendChild(button);
          Load_Passerelle_to_canvas(passerelle);
        }

       console.log("Traite Comments: "+Response.comments.length);
       for (var i = 0; i < Response.comments.length; i++)                            /* Pour chacun des liens, parsing un par un */
        { var comment = Response.comments[i];
          Load_Comment_to_canvas(comment);
        }

     };
    xhr.send();

    if (window.location.hostname=="test.watchdog.fr")
         var WTDWebSocket = new WebSocket("ws://test.watchdog.fr/ws/live-motifs", "live-motifs");
    else var WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/ws/live-motifs", "live-motifs");
    WTDWebSocket.onopen = function (event)
     { console.log("Connecté au websocket !");
       this.send ( JSON.stringify( { syn_id: id } ) );
     }
    WTDWebSocket.onerror = function (event)
     { console.log("Error au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onclose = function (event)
     { console.log("Close au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onmessage = function (event)
     { console.log("Recu WS Motif "+event.data);
       var Response = JSON.parse(event.data);                                               /* Pointe sur <synoptique a=1 ..> */
       Liste = document.getElementsByClassName("WTDCtrl_bit_" + Response.tech_id + "_" + Response.acronyme);
       for (const svg of Liste)
        { svg.SetState ( Response.mode, Response.color, Response.cligno ); }
     }
  }

/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_motif ( svg, event )
  { console.log(" Clic sur motif " + svg.motif.libelle + " icone_id = " + svg.motif.icone +
                "target techid/acro: " + svg.motif.clic_tech_id + ":" + svg.motif.clic_acronyme);
    var xhr = new XMLHttpRequest;
    xhr.open('get',base_url + "syn/clic/" + svg.motif.clic_tech_id + "/" + svg.motif.clic_acronyme, true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
     };
    xhr.send();
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Motif_to_canvas ( Motif )
  {
    var request = new XMLHttpRequest();                         /* Envoie une requete de récupération du SVG associé au motif */
    request.open("GET", "https://icons.abls-habitat.fr/assets/gif/"+Motif.forme+".svg", true);
    /*request.open("GET", "http://test.watchdog.fr/assets/gif/Jauge_2.svg", true);*/
    request.onreadystatechange = function()
     { if (request.readyState == 4 && (request.status == 200 || request.status == 0))
       { var svg = request.responseXML.documentElement;
         svg.motif = Motif;                                                            /* Sauvegarde du pointeur Motif source */

         svg.SetMode = new Function ( "mode", "color", svg.getElementById("SetMode").innerHTML );

         var mytitle=document.createElementNS("http://www.w3.org/2000/svg", 'title');
         mytitle.innerHTML = Motif.libelle;
         svg.appendChild(mytitle);

         svg.setAttribute( "id", "motif-"+svg.motif.id );
         svg.setAttribute( "class", "WTDCtrl_bit_" + Motif.tech_id + "_" + Motif.acronyme ); /* Affectation du control bit */
         if (Motif.def_color != null) svg.currentColor  = Motif.def_color;
                                 else svg.currentColor  = "#c8c8c8";
         svg.currentMode   = 0;
         svg.currentCligno = 0;
         svg.UpdateSVGMatrix = function ()                                       /* Fonction de mise a jour du positionnement */
           { var matrice="";                                                        /* Calcul de la matrice de transformation */
             var centrex = this.width.baseVal.value/2;
             var centrey = this.height.baseVal.value/2;
             matrice = matrice + "translate("+ this.motif.posx +"," + this.motif.posy +") ";
             matrice = matrice + "rotate("+ this.motif.angle + " 0 0) ";
             matrice = matrice + "scale("+this.motif.scale+") ";                    /* Scale, puis rotation, puis translation */
             matrice = matrice + "translate(-"+ centrex +",-" + centrey +") ";
             this.setAttribute( "transform", matrice );                                          /* Affectation de la matrice */
           };
         svg.SetState = function ( mode, color, cligno )                                 /* Fonction de changement d'etat */
           { if (color==undefined) color=svg.motif.def_color;
             this.SetMode ( mode );
             this.SetColor ( color );
             this.SetCligno ( cligno );
             /*console.log("SVG SetState for "+svg.motif.libelle+": mode "+mode+" color "+color+" cligno "+cligno);*/
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

         svg.SetCligno = function ( cligno )
           { if (cligno==this.currentCligno) return;
             if (cligno==1)
              { svg.currentCligno=1;
                svg.getElementById("fadeout").beginElement();
              }
             else
              { svg.currentCligno=0;
              }
           }

          svg.SetColor = function ( color )
           { targets = this.getElementsByClassName('WTD-ActiveFillColor');
             for (target of targets)
              { var chg_matrice = { fill: [ this.currentColor, color ] };                                   /* Matrice de changement */
                var chg_timing  = { duration: 500, fill: 'forwards' };                                       /* Timing de changement */
                target.animate ( chg_matrice, chg_timing );                                                          /* Go Animate ! */
              }

             targets = this.getElementsByClassName('WTD-ActiveStrokeColor');
             for (target of targets)
              { var chg_matrice = { stroke: [ this.currentColor, color ] };                                 /* Matrice de changement */
                var chg_timing  = { duration: 500, fill: 'forwards' };                                       /* Timing de changement */
                target.animate ( chg_matrice, chg_timing );                                                          /* Go Animate ! */
              }
             this.currentColor = color;
          }
                                                                                /* Connexion aux listeners  */
         svg.addEventListener ( "click", function (event) { Clic_sur_motif( svg, event ) }, false);
         svg.UpdateSVGMatrix();                     /* Mise a jour du SVG en fonction des parametres de positionnements Motif */
         $("#TopSVG").append(svg);                                                            /* ajout du SVG dans le Top SVG */
         setTimeout(function(){ svg.SetState ( Motif.mode, Motif.color, Motif.cligno ); }, 500);
       }
     };
    request.send(null);
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Passerelle_to_canvas ( Pass )
  { var svg   = document.createElementNS("http://www.w3.org/2000/svg", 'svg');
    svg.passerelle = Pass;                                                             /* Sauvegarde du pointeur Motif source */

    var texte = document.createElementNS("http://www.w3.org/2000/svg", 'text');
    texte.setAttribute("x",svg.passerelle.posx);
    texte.setAttribute("y",svg.passerelle.posy);
    texte.setAttribute("fill","red");
    texte.setAttribute("text-anchor","left");
    texte.setAttribute("font-family","courier");
    texte.setAttribute("font-size","20");
    texte.textContent = svg.passerelle.page;
    svg.append(texte);                                                                        /* ajout du SVG dans le Top SVG */

    var Motif = new Object;
    Motif.forme="auto_manu";
    Motif.libelle = "Vignette Activité";
    Motif.posx = parseInt(svg.passerelle.posx)+5;
    Motif.posy = parseInt(svg.passerelle.posy)-25;
    Motif.scale = 0.4;
    Motif.angle = 0;
    Motif.def_color = "#c8c8c8";
    Load_Motif_to_canvas(Motif);

    var Motif = new Object;
    Motif.forme="bouclier";
    Motif.libelle = "Vignette Sécurité Bien";
    Motif.posx = parseInt(svg.passerelle.posx)+25;
    Motif.posy = parseInt(svg.passerelle.posy)-25;
    Motif.scale = 0.4;
    Motif.angle = 0;
    Motif.def_color = "#c8c8c8";
    Load_Motif_to_canvas(Motif);

    if(svg.passerelle.angle == undefined) svg.passerelle.angle=0;
    svg.setAttribute("transform","rotate("+svg.passerelle.angle+" "+svg.passerelle.posx+" "+svg.passerelle.posy+")");
    svg.addEventListener ( "click", function (event) { Charger_syn(this.passerelle.syn_cible_id); }, false);
   /* svg.UpdateSVGMatrix();                     /* Mise a jour du SVG en fonction des parametres de positionnements Motif */
    console.debug(svg);
    $("#TopSVG").append(svg);                                                                 /* ajout du SVG dans le Top SVG */
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Lien_to_canvas ( Lien )
  { var svg=document.createElementNS("http://www.w3.org/2000/svg", 'line');
    svg.lien = Lien;                                                                   /* Sauvegarde du pointeur Motif source */
    svg.setAttribute("id", "WTD-lien-"+svg.lien.id);
    svg.setAttribute("x1", svg.lien.src_posx);
    svg.setAttribute("y1", svg.lien.src_posy);
    svg.setAttribute("x2", svg.lien.dst_posx);
    svg.setAttribute("y2", svg.lien.dst_posy);
    svg.setAttribute("stroke", svg.lien.stroke);
    svg.setAttribute("stroke-width", svg.lien.stroke_width);
    svg.setAttribute("stroke-linecap", svg.lien.stroke_linecap);
    if(svg.lien.stroke_dasharray!=undefined) svg.setAttribute("stroke-dasharray",svg.lien.stroke_dasharray);
    /*svg.addEventListener ( "click", function (event) { Clic_sur_lien( event ) }, false);*/
    $("#TopSVG").append(svg);                                                        /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Lien id: "+svg.lien.id);
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Rectangle_to_canvas ( Rectangle )
  { var svg=document.createElementNS("http://www.w3.org/2000/svg", 'rect');
    svg.rectangle = Rectangle;                                                                   /* Sauvegarde du pointeur Motif source */
    svg.setAttribute("id", "WTD-rectangle-"+svg.rectangle.id);
    svg.setAttribute("x", svg.rectangle.posx);
    svg.setAttribute("y", svg.rectangle.posy);
    svg.setAttribute("rx", svg.rectangle.rx);
    svg.setAttribute("ry", svg.rectangle.ry);
    svg.setAttribute("width", svg.rectangle.width);
    svg.setAttribute("height", svg.rectangle.height);
    svg.setAttribute("fill", svg.rectangle.color);
    svg.setAttribute("stroke", svg.rectangle.stroke);
    svg.setAttribute("stroke-width", svg.rectangle.stroke_width);
    if(svg.rectangle.stroke_dasharray!=undefined) svg.setAttribute("stroke-dasharray",svg.rectangle.stroke_dasharray);
    /*svg.addEventListener ( "click", function (event) { Clic_sur_rectangle( event ) }, false);*/
    $("#TopSVG").append(svg);                                                        /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Rectangle id: "+svg.rectangle.id);
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Comment_to_canvas ( Comment )
  { var svg=document.createElementNS("http://www.w3.org/2000/svg", 'text');
    svg.comment = Comment;                                                             /* Sauvegarde du pointeur Motif source */
    svg.setAttribute( "id", "WTD-comment-"+svg.comment.id );
    svg.setAttribute("x",svg.comment.posx);
    svg.setAttribute("y",svg.comment.posy);
    svg.setAttribute("fill",svg.comment.def_color);
    svg.setAttribute("text-anchor","middle");
    if(svg.comment.font       !=undefined) svg.setAttribute("font-family",svg.comment.font);
    if(svg.comment.font_size  !=undefined) svg.setAttribute("font-size",svg.comment.font_size);
    if(svg.comment.angle      ==undefined) svg.comment.angle=0;
    svg.setAttribute("transform","rotate("+svg.comment.angle+" "+svg.comment.posx+" "+svg.comment.posy+")");
    svg.textContent = svg.comment.libelle;

    svg.SetState = function ( state, color, cligno )                                      /* Fonction de changement d'etat */
     { if (color==undefined) color=this.comment.def_color;
       this.setAttribute("fill", color );                                                                     /* Go Animate ! */
       /*this.currentColor = color;
       this.ChangeCligno ( cligno );*/
     }
    /*svg.addEventListener ( "click", function (event) { Clic_sur_comment( event ) }, false);*/
    $("#TopSVG").append(svg);                                                                 /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Comment id: "+svg.comment.id);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
