 document.addEventListener('DOMContentLoaded', Load_page, false);

/********************************************* Appeler quand l'utilisateur selectionne un motif *******************************/
 function Clic_sur_motif ( svg, event )
  { console.log(" Clic sur motif " + svg.motif.libelle + " icone_id = " + svg.motif.icone +
                "target techid/acro: " + svg.motif.clic_tech_id + ":" + svg.motif.clic_acronyme);
    var json_request = JSON.stringify( { tech_id: svg.motif.clic_tech_id, acronyme: svg.motif.clic_acronyme } );
    Send_to_API ( 'POST', "/api/syn/clic", json_request, null, null );
  }

/********************************************* Appelé au chargement de la page ************************************************/
 function Load_page ()
  { console.log ("in load page !");

    vars = window.location.pathname.split('/');
    if ( vars[3] == undefined ) target=1; else target = vars[3];
    var json_request = JSON.stringify( { syn_id : target } );

    var WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/api/live-motifs", "live-motifs");
    WTDWebSocket.onopen = function (event)
     { var json_request = JSON.stringify( { wtd_session: localStorage.getItem("wtd_session"),
                                            ws_msg_type: "get_synoptique",
                                            syn_id : target }
                                        );
       console.log("WSOpen" + json_request );
       this.send ( json_request );
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
     { var Response = JSON.parse(event.data);                                               /* Pointe sur <synoptique a=1 ..> */
       if (Response.ws_msg_type == "update_visuel")
        { Visuel_Set_State ( Response.tech_id, Response.acronyme, Response.mode, Response.color, Response.cligno ); }
       else if (Response.ws_msg_type == "init_syn")
        { Init_syn ( false, Response ); }
       else console.log("ws_msg_type: " + Response.ws_msg_type + " not known");
     }
  }

