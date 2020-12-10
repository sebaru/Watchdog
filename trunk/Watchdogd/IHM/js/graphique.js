
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Gif_to_canvas ( Motif )
  {
    var svg = document.createElementNS("http://www.w3.org/2000/svg","image");
    svg.setAttribute("xlink","http://www.w3.org/1999/xlink");
    svg.setAttribute("href","https://icons.abls-habitat.fr/assets/gif/"+Motif.icone+".gif");
    svg.setAttribute("x", Motif.posx - Motif.larg/2 );
    svg.setAttribute("y", Motif.posy - Motif.haut/2 );
    svg.setAttribute("height", Motif.haut );
    svg.setAttribute("width", Motif.larg );
    svg.setAttribute("id","WTD-motif-"+Motif.id);
    svg.setAttribute("preserveAspectRatio","none");
    svg.setAttribute( "class", "svg-button" );
    if (Motif.gestion==1) $("#TopSVG").prepend(svg);                                                                 /* ajout du SVG dans le Top SVG */
    else $("#TopSVG").append(svg);                                                                 /* ajout du SVG dans le Top SVG */

  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Motif_to_canvas ( mode_atelier, Motif, init_mode, init_color, init_cligno )
  {
    var request = new XMLHttpRequest();                         /* Envoie une requete de récupération du SVG associé au motif */
    request.open("GET", "/img/"+Motif.forme+".svg", true);
    request.onreadystatechange = function()
     { if (request.readyState == 4 && (request.status == 200 || request.status == 0))
       { var svg = request.responseXML.documentElement;
         svg.motif = Motif;                                                            /* Sauvegarde du pointeur Motif source */

         if (svg.getElementById("SetMode") === null)
          { svg.SetMode = new Function ( "mode", "color", "return('false');"); }
         else
          { svg.SetMode = new Function ( "mode", "color", svg.getElementById("SetMode").innerHTML ); }

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
             console.log("SVG SetState for "+svg.motif.libelle+": mode "+mode+" color "+color+" cligno "+cligno);
             this.SetMode ( mode );
             this.SetColor ( color );
             this.SetCligno ( cligno );
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
         myanim.setAttribute("begin","indefinite");
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
         svg.SetState ( init_mode, init_color, init_cligno );
       }
     };
    request.send(null);
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Passerelle_to_canvas ( mode_atelier, Passerelle )
  { var svg   = document.createElementNS("http://www.w3.org/2000/svg", 'svg');
    svg.passerelle = Passerelle;                                                       /* Sauvegarde du pointeur Motif source */

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
    Load_Motif_to_canvas( mode_atelier, Motif, 0, Motif.def_color, 0 );

    var Motif = new Object;
    Motif.forme="bouclier";
    Motif.libelle = "Vignette Sécurité Bien";
    Motif.posx = parseInt(svg.passerelle.posx)+25;
    Motif.posy = parseInt(svg.passerelle.posy)-25;
    Motif.scale = 0.4;
    Motif.angle = 0;
    Motif.def_color = "#c8c8c8";
    Load_Motif_to_canvas( mode_atelier, Motif, 0, Motif.def_color, 0 );

    if(svg.passerelle.angle == undefined) svg.passerelle.angle=0;
    svg.setAttribute("transform","rotate("+svg.passerelle.angle+" "+svg.passerelle.posx+" "+svg.passerelle.posy+")");
    if (mode_atelier==false)
     { svg.addEventListener ( "click", function (event) { Redirect("/home/syn/"+this.passerelle.syn_cible_id); }, false); }
   /* svg.UpdateSVGMatrix();                     /* Mise a jour du SVG en fonction des parametres de positionnements Motif */
    console.debug(svg);
    $("#TopSVG").append(svg);                                                                 /* ajout du SVG dans le Top SVG */
  }
/********************************************* Prepare un objet SVG et l'affiche sur la page **********************************/
 function Load_Lien_to_canvas ( mode_atelier, Lien )
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
 function Load_Rectangle_to_canvas ( mode_atelier, Rectangle )
  { var svg=document.createElementNS("http://www.w3.org/2000/svg", 'rect');
    console.debug(Rectangle);
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
 function Load_Comment_to_canvas ( mode_atelier, Comment )
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

/********************************************* Créé un bouton bootstap sur SVG ************************************************/
 function New_svg_button( mode_atelier, color, icon, x, y, texte, on_click )
  { var svg = document.createElementNS("http://www.w3.org/2000/svg", "foreignObject");
    svg.setAttribute("x","0");
    svg.setAttribute("y","0");
    svg.setAttribute("width","100%");
    svg.setAttribute("height","100%");
    var button = document.createElementNS("http://www.w3.org/1999/xhtml","html:button");
    button.setAttribute("style","position: relative; top:"+y+"px; left:"+x+"px;");
    button.type = "button";
    button.innerHTML = "<i class='fas fa-"+icon+"'></i> "+texte;
    button.setAttribute("class","btn btn-sm btn-"+color);
    svg.append(button);
    return(svg);
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Init_syn ( mode_atelier, Response )
  { for (var i = 0; i < Response.motifs.length; i++)                             /* Pour chacun des motifs, parsing un par un */
     { var motif = Response.motifs[i];
       if (motif.icone==218 || motif.icone==222 || motif.icone==221)
        { }
       else if (motif.icone==424)
        { var button = New_svg_button ( mode_atelier, "outline-primary", "volume-mute", motif.posx-27, motif.posy-12, "Stop", null );
          $("#TopSVG").append(button);                                                     /* ajout du SVG dans le Top SVG */
        }
       else if (motif.icone==570)
        { var button = New_svg_button ( mode_atelier, "danger", "volume-up", motif.posx-32, motif.posy-12, "Panic", null );
          $("#TopSVG").append(button);                                                     /* ajout du SVG dans le Top SVG */
        }
       else if (motif.icone==217)
        { if (motif.angle==0)
           { var rectangle = { id: motif.id, posx:motif.posx-8, posy:motif.posy-8, rx: 1.0, ry: 1.0, width: 60, height: 60,
                               color: "none", stroke: "black", stroke_width: 5 };
             Load_Rectangle_to_canvas( mode_atelier, rectangle );                             /* ajout du SVG dans le Top SVG */
           }
        }
       else if (motif.icone==215)
        { if (motif.angle==-180)
           { var rectangle = { id: motif.id, posx:motif.posx-10, posy:motif.posy-10, rx: 1.0, ry: 1.0, width: 60, height: 60,
                               color: "none", stroke: "black", stroke_width: 5 };
             console.debug(rectangle);
             Load_Rectangle_to_canvas( mode_atelier, rectangle );                             /* ajout du SVG dans le Top SVG */
           }
        }
       else if (motif.forme=="none") Load_Gif_to_canvas ( motif );
       else
        { for (var j = 0; j < Response.visuels.length; j++)                     /* Pour chacun des visuels, parsing un par un */
           { var visuel = Response.visuels[j];
             if (visuel.tech_id == motif.tech_id && visuel.acronyme==motif.acronyme)
              { Load_Motif_to_canvas ( mode_atelier, motif, visuel.mode, visuel.color, visuel.cligno ); break; }
           }
          if (j==Response.visuels.length) /* si pas trouvé */
           { Load_Motif_to_canvas ( mode_atelier, motif, 0, motif.def_color, 0 ); }
        }
     }

    console.log("Traite Lien: "+Response.liens.length);
    for (var i = 0; i < Response.liens.length; i++)                            /* Pour chacun des liens, parsing un par un */
     { var lien = Response.liens[i];
       Load_Lien_to_canvas( mode_atelier, lien );
     }
    console.log("Traite Rectangle: "+Response.rectangles.length);
    for (var i = 0; i < Response.rectangles.length; i++)                            /* Pour chacun des liens, parsing un par un */
     { var rectangle = Response.rectangles[i];
       Load_Rectangle_to_canvas( mode_atelier, rectangle );
     }

    for (var i = 0; i < Response.passerelles.length; i++)                       /* Pour chacun des motifs, parsing un par un */
     { var passerelle = Response.passerelles[i];
       Load_Passerelle_to_canvas( mode_atelier, passerelle );
     }

    console.log("Traite Comments: "+Response.comments.length);
    for (var i = 0; i < Response.comments.length; i++)                            /* Pour chacun des liens, parsing un par un */
     { var comment = Response.comments[i];
       Load_Comment_to_canvas( mode_atelier, comment );
     }
  }

/******************************************************************************************************************************/
 function Visuel_Set_State ( tech_id, acronyme, mode, color, cligno )
  { console.log("Visuel Set State "+tech_id+":"+acronyme+" mode:"+mode+" color:"+color+" cligno:"+cligno );
    Liste = document.getElementsByClassName("WTDCtrl_bit_" + tech_id + "_" + acronyme);
    for (const svg of Liste)
     { svg.SetState ( mode, color, cligno ); }
  }
