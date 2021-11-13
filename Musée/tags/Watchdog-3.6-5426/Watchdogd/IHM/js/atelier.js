	 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_page ()
  { console.log ("in load archive !");
    vars = window.location.pathname.split('/');
    console.debug(vars);
    Charger_syn ( vars[3] )
	 }

 var svg_selected;

 var Div_Ctrl_panel_Motif=
       '<div class="row">'+
       '  <label class="col-4 col-form-label-sm">Posx</label>'+
       '  <input id="WTD-ctrl-panel-motif-posx" class="form-control-sm col-8" type="number" min="0" step="10" onchange="Change_motif_properties()"/>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-4 col-form-label-sm">Posy</label>'+
       '  <input id="WTD-ctrl-panel-motif-posy" class="form-control-sm col-8" type="number" min="0" step="10" onchange="Change_motif_properties()"/>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-4 col-form-label-sm">Scale</label>'+
       '  <input id="WTD-ctrl-panel-motif-scale" class="form-control-sm col-8" type="number" min="0.2" max="5" step="0.1" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-4 col-form-label-sm">Angle</label>'+
       '  <input id="WTD-ctrl-panel-motif-angle" class="form-control-sm col-8" type="number" min="0" max="360" step="5" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Default Color</label>'+
       '  <input id="WTD-ctrl-panel-motif-color" class="form-control-sm col-6" type="color" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Ctrl Tech_id</label>'+
       '  <input id="WTD-ctrl-panel-motif-tech-id" class="form-control-sm col-6" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Ctrl Acronyme</label>'+
       '  <input id="WTD-ctrl-panel-motif-acronyme" class="form-control-sm col-6" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Clic Tech_id</label>'+
       '  <input id="WTD-ctrl-panel-motif-clic-tech-id" class="form-control-sm col-6" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Clic Acronyme</label>'+
       '  <input id="WTD-ctrl-panel-motif-clic-acronyme" class="form-control-sm col-6" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-6 col-form-label-sm">Access Level</label>'+
       '  <input id="WTD-ctrl-panel-motif-access-level" class="form-control-sm col-6" type="number" min="0" max="9" step="1" onchange="Change_motif_properties()"/> </div>'+
       '</div>'+

       '<div class="row">'+
       '  <label class="col-4 col-form-label-sm">Libellé</label>'+
       '  <input id="WTD-ctrl-panel-motif-libelle" class="form-control-sm col-8" onchange="Change_motif_properties()"/> </div>'+
       '</div>';
 var Div_Ctrl_panel_Lien=
       '<div class="col-sd-1"><label>Src_Posx</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-x1" type="number" min="0" step="10" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Src_Posy</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-y1" type="number" min="0" step="10" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Dst_Posx</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-x2" type="number" min="0" step="10" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Dst_Posy</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-y2" type="number" min="0" step="10" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Stroke</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-stroke" type="color" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>StrokeWidth</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-stroke-width" type="number" min="1" max="15" step="1" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>DashArray</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-stroke-dasharray" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Linecap</label></div>'+
       '<div class="col-sd-1"><select id="WTD-ctrl-panel-lien-stroke-linecap" onchange="Change_lien_properties()"/>'+
                              '<option value="butt">Normal</option>'+
                              '<option value="round">Arrondi</option>'+
                              '<option value="square">Carré</option>'+
                              '</select></div>'+
       '<div class="col-sd-1"><label>Control Tech_id</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-tech-id" onchange="Change_lien_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Control Acronyme</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-lien-acronyme" onchange="Change_lien_properties()"/> </div>';
 var Div_Ctrl_panel_Rectangle=
       '<div class="col-sd-1"><label>Posx</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-posx" type="number" min="0" step="10" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Posy</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-posy" type="number" min="0" step="10" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Width</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-width" type="number" min="5" step="5" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Height</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-height" type="number" min="5" step="5" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Rx</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-rx" type="number" min="0" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Ry</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-ry" type="number" min="0" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Default color</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-def-color" type="color" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Stroke</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-stroke" type="color" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>StrokeWidth</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-stroke-width" type="number" min="1" max="15" step="1" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>DashArray</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-stroke-dasharray" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Control Tech_id</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-tech-id" onchange="Change_rectangle_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Control Acronyme</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-rectangle-acronyme" onchange="Change_rectangle_properties()"/> </div>';
 var Div_Ctrl_panel_Comment=
       '<div class="col-sd-1"><label>Libellé</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-libelle" type="text" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Posx</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-posx" type="number" min="0" step="10" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Posy</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-posy" type="number" min="0" step="10" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Couleur</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-def-color" type="color" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Angle</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-angle" type="number" min="0" max="360" step="5" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Font Family</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-font-family" onchange="Change_comment_properties()"/> </div>'+
       '<div class="col-sd-1"><label>Font size</label></div>'+
       '<div class="col-sd-1"><input id="WTD-ctrl-panel-comment-font-size" type="number" min="10" max="100" step="5" onchange="Change_comment_properties()"/> </div>';
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Ajouter_icone ()
  { var topsvg = document.getElementById("TopSVG");
    var object =
     { syn_id: syn_id, bitctrl: -1,
       icone: $("#select-icones").val(), libelle: "Nouveau motif",
       posx: 150, posy: 150, angle : 0, scale : 1, def_color : "#c8c8c8",
       tech_id : "", acronyme:"", clic_tech_id : "", clic_acronyme:"",
       access_level:0
     };
    var json_request = JSON.stringify(object);
    var xhr = new XMLHttpRequest;
    xhr.open('post',base_url + "admin/syn/add_motif", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       Load_Motif_to_canvas ( Response[0] );
     };
    xhr.send(json_request);
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_icone ()
  { var xhr = new XMLHttpRequest;
    xhr.open('get', "https://icons.abls-habitat.fr/icons/icon_list/"+$("#select-classes").val(), true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText, true);

       var options = "";
       for(var i = 0; i < Response.length; i++)
        { options += "<option value='" + Response[i][0] + "'>" + Response[i][1] + "</option>"; }
       $("#select-icones").find('option').remove().end().append($(options));
     };
    xhr.send();
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_classe ( )
  { var xhr = new XMLHttpRequest;
    xhr.open('get', "https://icons.abls-habitat.fr/icons/class_list", true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText, true);

       var options = "";
       for(var i = 0; i < Response.length; i++)
        { options += "<option value='" + Response[i][0] + "'>" + Response[i][1] + "</option>"; }
       $("#select-classes").find('option').remove().end().append($(options));
       Charger_icone();
     };
    xhr.send();
  }

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_syn ( id )
  { var topsvg = document.getElementById("TopSVG");
    if (!topsvg) return;
    syn_id = id;
    while (topsvg.firstChild) { topsvg.removeChild(topsvg.firstChild); }
    /*var listpass = document.getElementById("liste_passerelles");
    while (listpass.firstChild) { listpass.removeChild(listpass.firstChild); }*/
    var xhr = new XMLHttpRequest;
    xhr.open('get', "/api/syn/show/" + id, true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */

       console.log("Traite motifs: "+Response.motifs.length);
       for (var i = 0; i < Response.motifs.length; i++)                          /* Pour chacun des motifs, parsing un par un */
        { var motif = Response.motifs[i];
          Load_Motif_to_canvas ( motif );
        }

       /*var listpass = document.getElementById("liste_passerelles");                                 /* Pour chaque passerelle */
       /*var button = document.createElement('button');
       button.setAttribute( "class", "btn btn-primary" );
       button.innerHTML = "Accueil";
       button.onclick = function() { Charger_syn(1); }
       listpass.appendChild(button);
/*       for (var i = 0; i < Response.passerelles.length; i++)                       /* Pour chacun des motifs, parsing un par un */
/*        { var passerelle = Response.passerelles[i];
          var button = document.createElement('button');
          button.setAttribute( "class", "btn btn-secondary" );
          button.innerHTML = passerelle.page;
          button.onclick = function() { Charger_syn(passerelle.syn_cible_id); }
          listpass.appendChild(button);
        }*/

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

       console.log("Traite Comments: "+Response.comments.length);
       for (var i = 0; i < Response.comments.length; i++)                            /* Pour chacun des liens, parsing un par un */
        { var comment = Response.comments[i];
          Load_Comment_to_canvas(comment);
        }
     };
    xhr.send();
    Charger_classe();
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Deselectionner ( )
  { if (svg_selected)
     { if (svg_selected.motif)     { svg_selected.ChangeState ( 0, svg_selected.motif.def_color, false ); }
       if (svg_selected.rectangle) { svg_selected.ChangeState ( 0, svg_selected.rectangle.def_color, false ); }
       if (svg_selected.lien)      { svg_selected.ChangeState ( 0, svg_selected.lien.stroke, false ); }
       if (svg_selected.comment)   { svg_selected.ChangeState ( 0, svg_selected.comment.def_color, false ); }
     }
    document.getElementById("WTD-ctrl-panel").innerHTML="Rien n'est sélectionné";
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Sauvegarder_synoptique ()
  { var Motifs=[], Liens=[], Rectangles=[], Comments=[];
    var topsvg = document.getElementById("TopSVG");
    var svgs = Array.from(topsvg.childNodes);
    svgs.forEach( function(svg)
                   { if(svg.motif) Motifs.push( svg.motif );
                     if(svg.lien)  Liens.push( svg.lien );
                     if(svg.rectangle)  Rectangles.push( svg.rectangle );
                     if(svg.comment)  Comments.push( svg.comment );
                   } );

    var json_request = JSON.stringify(Motifs);
    var xhr = new XMLHttpRequest;
    xhr.open('POST', "/api/syn/update_motifs", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idToastStatus').toast('show'); }
       else if (xhr.status == 401)
        { Show_Error ( "Vos identifiants et mots de passe sont incorrects" ); }
       else
        { Show_Error ( xhr.statusText ); }
     };
    xhr.send(json_request);

    var json_request = JSON.stringify(Liens);
    var xhr = new XMLHttpRequest;
    xhr.open('post', "/api/syn/update_liens", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idToastStatus').toast('show'); }
       else if (xhr.status == 401)
        { Show_Error ( "Vos identifiants et mots de passe sont incorrects" ); }
       else
        { Show_Error ( xhr.statusText ); }
     };
    xhr.send(json_request);

    var json_request = JSON.stringify(Rectangles);
    var xhr = new XMLHttpRequest;
    xhr.open('post', "/api/syn/update_rectangles", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idToastStatus').toast('show'); }
       else if (xhr.status == 401)
        { Show_Error ( "Vos identifiants et mots de passe sont incorrects" ); }
       else
        { Show_Error ( xhr.statusText ); }
     };
    xhr.send(json_request);

    var json_request = JSON.stringify(Comments);
    var xhr = new XMLHttpRequest;
    xhr.open('post', "/api/syn/update_comments", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status == 200)
        { $('#idToastStatus').toast('show'); }
       else if (xhr.status == 401)
        { Show_Error ( "Vos identifiants et mots de passe sont incorrects" ); }
       else
        { Show_Error ( xhr.statusText ); }
     };
    xhr.send(json_request);

  }
/********************************************* Appeler quand l'utilisateur veut supprimer la selection ************************/
 function Supprimer ()
  { var json_request;
    var xhr = new XMLHttpRequest;
    if (svg_selected.motif)
     {  json_request = JSON.stringify(svg_selected.motif);
        xhr.open('post',base_url + "admin/syn/delete_motif", true);
     }
    if (svg_selected.lien)
     { json_request = JSON.stringify(svg_selected.lien);
       xhr.open('post',base_url + "admin/syn/delete_lien", true);
     }
    if (svg_selected.rectangle)
     { json_request = JSON.stringify(svg_selected.rectangle);
       xhr.open('post',base_url + "admin/syn/delete_rectangle", true);
     }
    if (svg_selected.comment)
     { json_request = JSON.stringify(svg_selected.comment);
       xhr.open('post',base_url + "admin/syn/delete_comment", true);
     }
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       svg_selected.parentNode.removeChild(svg_selected);
       Deselectionner();
     };
    xhr.send(json_request);
  }
/********************************************* Appeler quand l'utilisateur veut supprimer la selection ************************/
 function Dupliquer ()
  { var json_request;
    var xhr = new XMLHttpRequest;
    if (svg_selected.motif)
     { json_request = JSON.stringify(svg_selected.motif);
       xhr.open('post',base_url + "admin/syn/add_motif", true);
     }
    if (svg_selected.lien)
     { json_request = JSON.stringify(svg_selected.lien);
       xhr.open('post',base_url + "admin/syn/add_lien", true);
     }
    if (svg_selected.rectangle)
     { json_request = JSON.stringify(svg_selected.rectangle);
       xhr.open('post',base_url + "admin/syn/add_rectangle", true);
     }
    if (svg_selected.comment)
     { json_request = JSON.stringify(svg_selected.comment);
       xhr.open('post',base_url + "admin/syn/add_comment", true);
     }
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       if (svg_selected.motif)      Load_Motif_to_canvas ( Response[0] );
       if (svg_selected.lien)       Load_Lien_to_canvas ( Response[0] );
       if (svg_selected.rectangle)  Load_Rectangle_to_canvas ( Response[0] );
       if (svg_selected.comment)    Load_Comment_to_canvas ( Response[0] );
     };
    xhr.send(json_request);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Change_motif_properties ()
  { svg_selected.motif.posx          = document.getElementById("WTD-ctrl-panel-motif-posx").value;
    svg_selected.motif.posy          = document.getElementById("WTD-ctrl-panel-motif-posy").value;
    svg_selected.motif.angle         = document.getElementById("WTD-ctrl-panel-motif-angle").value;
    svg_selected.motif.scale         = document.getElementById("WTD-ctrl-panel-motif-scale").value;
    svg_selected.motif.def_color     = document.getElementById("WTD-ctrl-panel-motif-color").value;
    svg_selected.motif.access_level  = document.getElementById("WTD-ctrl-panel-motif-access-level").value;
    svg_selected.motif.libelle       = document.getElementById("WTD-ctrl-panel-motif-libelle").value;
    svg_selected.motif.tech_id       = document.getElementById("WTD-ctrl-panel-motif-tech-id").value;
    svg_selected.motif.acronyme      = document.getElementById("WTD-ctrl-panel-motif-acronyme").value;
    svg_selected.motif.clic_tech_id  = document.getElementById("WTD-ctrl-panel-motif-clic-tech-id").value;
    svg_selected.motif.clic_acronyme = document.getElementById("WTD-ctrl-panel-motif-clic-acronyme").value;
    svg_selected.UpdateSVGMatrix();
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function svgPoint(element, x, y)
  { var pt = document.getElementById("TopSVG").createSVGPoint();
    pt.x = x;
    pt.y = y;
    return pt.matrixTransform(element.getScreenCTM().inverse());
  }

/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Down_sur_motif ( svg, event )
  { console.log(" Down sur motif " + svg.motif.libelle + " offsetx = " + event.clientX + " offsetY="+event.clientY );
    svg.motif.selected = true;

    if (svg_selected != undefined)
     { console.log("svg_selected =" + svg_selected.motif);
       if (svg.motif.id != svg_selected.motif.id) Deselectionner();
     }

    console.log(" Clic sur motif " + svg.motif.libelle + " icone_id = " + svg.motif.icone );
    /*console.debug(svg);*/
    document.getElementById("WTD-ctrl-panel").innerHTML= Div_Ctrl_panel_Motif;
    document.getElementById("WTD-ctrl-panel-motif-access-level").value  = svg.motif.access_level;
    document.getElementById("WTD-ctrl-panel-motif-libelle").value       = svg.motif.libelle;
    document.getElementById("WTD-ctrl-panel-motif-tech-id").value       = svg.motif.tech_id;
    document.getElementById("WTD-ctrl-panel-motif-acronyme").value      = svg.motif.acronyme;
    document.getElementById("WTD-ctrl-panel-motif-clic-tech-id").value  = svg.motif.clic_tech_id;
    document.getElementById("WTD-ctrl-panel-motif-clic-acronyme").value = svg.motif.clic_acronyme;
    document.getElementById("WTD-ctrl-panel-motif-posx").value          = svg.motif.posx;
    document.getElementById("WTD-ctrl-panel-motif-posy").value          = svg.motif.posy;
    document.getElementById("WTD-ctrl-panel-motif-angle").value         = svg.motif.angle;
    document.getElementById("WTD-ctrl-panel-motif-scale").value         = svg.motif.scale;
    document.getElementById("WTD-ctrl-panel-motif-color").value         = svg.motif.def_color;
    svg_selected = svg;
    svg_selected.ChangeState ( 0, "#0000dd", 0 );

  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Move_sur_motif ( svg, event )
  { if (svg.motif.selected)
     { console.log(" Clic sur motif " + svg.motif.libelle + " offsetx = " + event.clientX + " offsetY="+event.clientY );
       target = svgPoint ( document.getElementById("TopSVG"), event.clientX, event.clientY );
       svg.motif.posx = target.x.toString();
       svg.motif.posy = target.y.toString();
       svg.UpdateSVGMatrix ();
     }
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Up_sur_motif ( svg, event )
  { console.log(" Up sur motif " + svg.motif.libelle + " offsetx = " + event.clientX + " offsetY="+event.clientY );
    svg.motif.selected = false;
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
         svg.setAttribute( "class", "WTDControl_bit_" + Motif.bitctrl );                        /* Affectation du control bit */
         svg.currentColor  = Motif.def_color;
         svg.currentState  = 0;
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
         svg.addEventListener ( "click", function (event) { Clic_sur_motif ( svg ) }, false);
         svg.addEventListener ( "mouseup", function (event) { Up_sur_motif( svg, event ) }, false);
         /*svg.addEventListener ( "mouseleave", function (event) { Up_sur_motif( svg, event ) }, false);*/
         svg.addEventListener ( "mousedown", function (event) { Down_sur_motif( svg, event ) }, false);
         svg.addEventListener ( "mousemove", function (event) { Move_sur_motif( svg, event ) }, false);
         svg.UpdateSVGMatrix();                     /* Mise a jour du SVG en fonction des parametres de positionnements Motif */
         $("#TopSVG").append(svg);                                                            /* ajout du SVG dans le Top SVG */
         setTimeout(function(){ svg.ChangeState ( 0, Motif.def_color, false ); }, 500);
       }
     };
    request.send(null);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Change_lien_properties ()
  { svg_selected.lien.src_posx = document.getElementById("WTD-ctrl-panel-lien-x1").value;
    svg_selected.setAttribute("x1", svg_selected.lien.src_posx);
    svg_selected.lien.src_posy = document.getElementById("WTD-ctrl-panel-lien-y1").value;
    svg_selected.setAttribute("y1", svg_selected.lien.src_posy);
    svg_selected.lien.dst_posx = document.getElementById("WTD-ctrl-panel-lien-x2").value;
    svg_selected.setAttribute("x2", svg_selected.lien.dst_posx);
    svg_selected.lien.dst_posy = document.getElementById("WTD-ctrl-panel-lien-y2").value;
    svg_selected.setAttribute("y2", svg_selected.lien.dst_posy);
    svg_selected.lien.stroke = document.getElementById("WTD-ctrl-panel-lien-stroke").value;
    svg_selected.setAttribute("stroke", svg_selected.lien.stroke);
    svg_selected.lien.stroke_width = document.getElementById("WTD-ctrl-panel-lien-stroke-width").value;
    svg_selected.setAttribute("stroke-width", svg_selected.lien.stroke_width);
    svg_selected.lien.stroke_dasharray = document.getElementById("WTD-ctrl-panel-lien-stroke-dasharray").value;
    svg_selected.setAttribute("stroke-dasharray", svg_selected.lien.stroke_dasharray);
    svg_selected.lien.stroke_linecap = document.getElementById("WTD-ctrl-panel-lien-stroke-linecap").value;
    svg_selected.setAttribute("stroke-linecap", svg_selected.lien.stroke_linecap);
    svg_selected.lien.tech_id  = document.getElementById("WTD-ctrl-panel-lien-tech-id").value;
    svg_selected.lien.acronyme = document.getElementById("WTD-ctrl-panel-lien-acronyme").value;
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Ajouter_lien ()
  { var topsvg = document.getElementById("TopSVG");
    var object =
     { syn_id: syn_id,
       src_posx: 100,
       src_posy: 100,
       dst_posx: 200,
       dst_posy: 200,
       stroke: "#c8c8c8",
       stroke_dasharray: "",
       stroke_linecap: "butt",
       stroke_width: 5,
       tech_id: "",
       acronyme: ""
     };
    var json_request = JSON.stringify(object);
    var xhr = new XMLHttpRequest;
    xhr.open('post',base_url + "admin/syn/add_lien", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       Load_Lien_to_canvas ( Response[0] );
     };
    xhr.send(json_request);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_lien ( event )
  { Deselectionner();
    svg=event.target;
    console.debug(svg);
    console.log(" Clic sur lien " + svg.lien.id );
    document.getElementById("WTD-ctrl-panel").innerHTML= Div_Ctrl_panel_Lien;
    document.getElementById("WTD-ctrl-panel-lien-x1").value = svg.lien.src_posx;
    document.getElementById("WTD-ctrl-panel-lien-y1").value = svg.lien.src_posy;
    document.getElementById("WTD-ctrl-panel-lien-x2").value = svg.lien.dst_posx;
    document.getElementById("WTD-ctrl-panel-lien-y2").value = svg.lien.dst_posy;
    document.getElementById("WTD-ctrl-panel-lien-stroke").value = svg.lien.stroke;
    document.getElementById("WTD-ctrl-panel-lien-stroke-width").value = svg.lien.stroke_width;
    document.getElementById("WTD-ctrl-panel-lien-stroke-dasharray").value = svg.lien.stroke_dasharray;
    document.getElementById("WTD-ctrl-panel-lien-stroke-linecap").value = svg.lien.stroke_linecap;
    document.getElementById("WTD-ctrl-panel-lien-tech-id").value = svg.lien.tech_id;
    document.getElementById("WTD-ctrl-panel-lien-acronyme").value = svg.lien.acronyme;
    svg_selected = svg;
    svg_selected.ChangeState ( 0, "#0000dd", 0 );
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

    svg.ChangeState = function ( state, color, cligno )                                      /* Fonction de changement d'etat */
     { if (color==undefined) color=this.lien.stroke;
       this.setAttribute("stroke", color );                                                                   /* Go Animate ! */
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

    console.debug(svg);
    svg.addEventListener ( "click", function (event) { Clic_sur_lien( event ) }, false);
    $("#TopSVG").append(svg);                                                        /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Lien id: "+svg.lien.id);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Ajouter_rectangle ()
  { var topsvg = document.getElementById("TopSVG");
    var object =
     { syn_id: syn_id,
       posx: 100,
       posy: 100,
       width: 10,
       height: 10,
       rx: 3,
       ry: 3,
       stroke: "#00ff00",
       stroke_width: 1,
       def_color: "#c8c8c8",
       tech_id : "",
       acronyme:""
     };
    var json_request = JSON.stringify(object);
    var xhr = new XMLHttpRequest;
    xhr.open('post',base_url + "admin/syn/add_rectangle", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       Load_Rectangle_to_canvas ( Response[0] );
     };
    xhr.send(json_request);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Change_rectangle_properties ()
  { svg_selected.rectangle.posx = document.getElementById("WTD-ctrl-panel-rectangle-posx").value;
    svg_selected.setAttribute("x", svg_selected.rectangle.posx);
    svg_selected.rectangle.posy = document.getElementById("WTD-ctrl-panel-rectangle-posy").value;
    svg_selected.setAttribute("y", svg_selected.rectangle.posy);
    svg_selected.rectangle.width = document.getElementById("WTD-ctrl-panel-rectangle-width").value;
    svg_selected.setAttribute("width", svg_selected.rectangle.width);
    svg_selected.rectangle.height = document.getElementById("WTD-ctrl-panel-rectangle-height").value;
    svg_selected.setAttribute("height", svg_selected.rectangle.height);
    svg_selected.rectangle.rx = document.getElementById("WTD-ctrl-panel-rectangle-rx").value;
    svg_selected.setAttribute("rx", svg_selected.rectangle.rx);
    svg_selected.rectangle.ry = document.getElementById("WTD-ctrl-panel-rectangle-ry").value;
    svg_selected.setAttribute("ry", svg_selected.rectangle.ry);
    svg_selected.rectangle.stroke = document.getElementById("WTD-ctrl-panel-rectangle-stroke").value;
    svg_selected.setAttribute("stroke", svg_selected.rectangle.stroke);
    svg_selected.rectangle.stroke_width = document.getElementById("WTD-ctrl-panel-rectangle-stroke-width").value;
    svg_selected.setAttribute("stroke-width", svg_selected.rectangle.stroke_width);
    svg_selected.rectangle.stroke_dasharray = document.getElementById("WTD-ctrl-panel-rectangle-stroke-dasharray").value;
    svg_selected.setAttribute("stroke-dasharray", svg_selected.rectangle.stroke_dasharray);
    svg_selected.rectangle.def_color = document.getElementById("WTD-ctrl-panel-rectangle-def-color").value;
    svg_selected.setAttribute("fill", svg_selected.rectangle.color);
    svg_selected.rectangle.tech_id  = document.getElementById("WTD-ctrl-panel-rectangle-tech-id").value;
    svg_selected.rectangle.acronyme = document.getElementById("WTD-ctrl-panel-rectangle-acronyme").value;
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_rectangle ( event )
  { Deselectionner();
    svg=event.target;
    console.debug(svg);
    console.log(" Clic sur rectangle " + svg.rectangle.id );
    document.getElementById("WTD-ctrl-panel").innerHTML= Div_Ctrl_panel_Rectangle;
    document.getElementById("WTD-ctrl-panel-rectangle-posx").value = svg.rectangle.posx;
    document.getElementById("WTD-ctrl-panel-rectangle-posy").value = svg.rectangle.posy;
    document.getElementById("WTD-ctrl-panel-rectangle-width").value = svg.rectangle.width;
    document.getElementById("WTD-ctrl-panel-rectangle-height").value = svg.rectangle.height;
    document.getElementById("WTD-ctrl-panel-rectangle-rx").value = svg.rectangle.rx;
    document.getElementById("WTD-ctrl-panel-rectangle-ry").value = svg.rectangle.ry;
    document.getElementById("WTD-ctrl-panel-rectangle-stroke").value = svg.rectangle.stroke;
    document.getElementById("WTD-ctrl-panel-rectangle-stroke-width").value = svg.rectangle.stroke_width;
    document.getElementById("WTD-ctrl-panel-rectangle-stroke-dasharray").value = svg.rectangle.stroke_dasharray;
    document.getElementById("WTD-ctrl-panel-rectangle-def-color").value = svg.rectangle.def_color;
    document.getElementById("WTD-ctrl-panel-rectangle-tech-id").value = svg.rectangle.tech_id;
    document.getElementById("WTD-ctrl-panel-rectangle-acronyme").value = svg.rectangle.acronyme;
    svg_selected = svg;
    svg_selected.ChangeState ( 0, "#0000dd", 0 );
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
    svg.setAttribute("fill", svg.rectangle.def_color);
    svg.setAttribute("stroke", svg.rectangle.stroke);
    svg.setAttribute("stroke-width", svg.rectangle.stroke_width);
    if(svg.rectangle.stroke_dasharray!=undefined) svg.setAttribute("stroke-dasharray",svg.rectangle.stroke_dasharray);

    svg.ChangeState = function ( state, color, cligno )                                      /* Fonction de changement d'etat */
     { if (color==undefined) color=this.rectangle.def_color;
       this.setAttribute("fill", color );                                                                     /* Go Animate ! */
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
    console.debug(svg);
    svg.addEventListener ( "click", function (event) { Clic_sur_rectangle( event ) }, false);
    $("#TopSVG").append(svg);                                                        /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Rectangle id: "+svg.rectangle.id);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Change_comment_properties ()
  { svg_selected.comment.libelle = document.getElementById("WTD-ctrl-panel-comment-libelle").value;
    svg_selected.textContent = svg_selected.comment.libelle;
    svg_selected.comment.posx = document.getElementById("WTD-ctrl-panel-comment-posx").value;
    svg_selected.setAttribute("x", svg_selected.comment.posx);
    svg_selected.comment.posy = document.getElementById("WTD-ctrl-panel-comment-posy").value;
    svg_selected.setAttribute("y", svg_selected.comment.posy);
    svg_selected.comment.angle = document.getElementById("WTD-ctrl-panel-comment-angle").value;
    svg_selected.setAttribute("transform","rotate("+svg.comment.angle+" "+svg.comment.posx+" "+svg.comment.posy+")");
    svg_selected.comment.def_color = document.getElementById("WTD-ctrl-panel-comment-def-color").value;
    svg_selected.setAttribute("fill", svg_selected.comment.def_color);
    svg_selected.comment.font = document.getElementById("WTD-ctrl-panel-comment-font-family").value;
    svg_selected.setAttribute("font-family", svg_selected.comment.font);
    svg_selected.comment.font_size = document.getElementById("WTD-ctrl-panel-comment-font-size").value;
    svg_selected.setAttribute("font-size", svg_selected.comment.font_size);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Ajouter_comment ()
  { var topsvg = document.getElementById("TopSVG");
    var object =
     { syn_id: syn_id,
       libelle: "New Comment !",
       posx: 100,
       posy: 100,
       def_color: "#c8c8c8",
       font: "Verdana",
       font_size: 20
     };
    var json_request = JSON.stringify(object);
    var xhr = new XMLHttpRequest;
    xhr.open('post',base_url + "admin/syn/add_comment", true);
    xhr.setRequestHeader('Content-type', 'application/json');
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       Load_Comment_to_canvas ( Response[0] );
     };
    xhr.send(json_request);
  }
/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_comment ( event )
  { Deselectionner();
    svg=event.target;
    console.debug(svg);
    console.log(" Clic sur Comment " + svg.comment.id );
    document.getElementById("WTD-ctrl-panel").innerHTML= Div_Ctrl_panel_Comment;
    document.getElementById("WTD-ctrl-panel-comment-libelle").value = svg.comment.libelle;
    document.getElementById("WTD-ctrl-panel-comment-posx").value = svg.comment.posx;
    document.getElementById("WTD-ctrl-panel-comment-posy").value = svg.comment.posy;
    document.getElementById("WTD-ctrl-panel-comment-angle").value = svg.comment.angle;
    document.getElementById("WTD-ctrl-panel-comment-def-color").value = svg.comment.def_color;
    document.getElementById("WTD-ctrl-panel-comment-font-family").value = svg.comment.font;
    document.getElementById("WTD-ctrl-panel-comment-font-size").value = svg.comment.font_size;
    svg_selected = svg;
    /*svg_selected.ChangeState ( 0, "#0000dd", 0 );*/
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

    svg.ChangeState = function ( state, color, cligno )                                      /* Fonction de changement d'etat */
     { if (color==undefined) color=this.comment.def_color;
       this.setAttribute("fill", color );                                                                     /* Go Animate ! */
       /*this.currentColor = color;
       this.ChangeCligno ( cligno );*/
     }
    svg.addEventListener ( "click", function (event) { Clic_sur_comment( event ) }, false);
    console.debug(svg);
    $("#TopSVG").append(svg);                                                                 /* ajout du SVG dans le Top SVG */
    console.log("Fin Traite Comment id: "+svg.comment.id);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
