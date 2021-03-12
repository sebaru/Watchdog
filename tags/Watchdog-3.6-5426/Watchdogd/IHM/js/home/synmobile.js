 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_motif ( svg, event )
  { console.log(" Clic sur motif " + svg.motif.libelle + " icone_id = " + svg.motif.icone +
                "target techid/acro: " + svg.motif.clic_tech_id + ":" + svg.motif.clic_acronyme);
    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/syn/clic/" + svg.motif.clic_tech_id + "/" + svg.motif.clic_acronyme, true);
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
    request.onreadystatechange = function()
     { if (request.readyState == 4 && (request.status == 200 || request.status == 0))
       { var svg = request.responseXML.documentElement;
         svg.motif = Motif;                                                            /* Sauvegarde du pointeur Motif source */
         if(Motif.icone==428) console.debug(Motif);
         svg.setAttribute( "id", "WTD-motif-"+svg.motif.id );
         svg.setAttribute( "class", "WTDCtrl_bit_"+ svg.motif.tech_id+"_"+svg.motif.acronyme ); /* Affectation du control bit */
         svg.setAttribute( "width", "60" );                                                     /* Affectation du control bit */
         svg.setAttribute( "height", "60" );                                                    /* Affectation du control bit */
         svg.currentColor  = Motif.def_color;
         svg.currentState  = 0;
         svg.currentCligno = 0;
         svg.SetState = function ( state, color, cligno )                                    /* Fonction de changement d'etat */
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
                                                                                                  /* Connexion aux listeners  */
         svg.addEventListener ( "click", function (event) { Clic_sur_motif ( svg ) }, false);
         $("#idSVG-"+svg.motif.id).append(svg);                                               /* ajout du SVG dans le Top SVG */
         setTimeout(function(){ svg.SetState ( 0, Motif.def_color, false ); }, 500);
       }
     };
    request.send(null);
  }
/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");

    vars = window.location.pathname.split('/');
    var json_request = JSON.stringify( { syn_id : vars[3] } );
    Send_to_API ( "PUT", "/api/syn/show", json_request, function(Response)
     { console.log("Traite motifs: "+Response.motifs.length);
       for (var i = 0; i < Response.motifs.length; i++)                          /* Pour chacun des motifs, parsing un par un */
        { var motif = Response.motifs[i];
          $('#idContainer').append("<div class='row border border-warning mb-1'>"+
                                     "<div id='idSVG-"+motif.id+"' class='col-3 p-1 justify-center' ></div>"+
                                     "<div class='col-90 align-self-center' ><p class='text-light'>"+motif.libelle+"</p></div>"+
                                   "</div>");
          Load_Motif_to_canvas ( motif );
        }
     }, null );

    var WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/api/live-motifs", "live-motifs");
    WTDWebSocket.onopen = function (event)
     { console.log("Connecté au websocket !");
       this.send ( JSON.stringify( { syn_id: vars[3] } ) );
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
