 document.addEventListener('DOMContentLoaded', Load_page, false);
 document.addEventListener('pageshow', Load_websocket, false);

 var WTDWebSocket;

 function Ping ()
  { setTimeout ( function()                                                                         /* Un ping tous les jours */
     { Send_to_API ( "GET", "/api/ping", null, function ()
        { if (WTDWebSocket && WTDWebSocket.readyState != "OPEN")
           { console.log("Ping : websocket status = " + WTDWebSocket.readyState );
             Load_websocket();
           }
          Ping();
        }, null );
     }, 60000 );
  }
/******************************************************************************************************************************/
/* Load_websocket: Appelé pour ouvrir la websocket                                                                            */
/******************************************************************************************************************************/
 function Load_websocket ()
  { if (WTDWebSocket && WTDWebSocket.readyState == "OPEN") return;
    WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/api/live-motifs", "live-motifs");
    WTDWebSocket.onopen = function (event)
     { $('#idAlertConnexionLost').hide();
       var json_request = JSON.stringify( { zmq_tag: "CONNECT", wtd_session: localStorage.getItem("wtd_session") } );
       this.send ( json_request );
     }
    WTDWebSocket.onerror = function (event)
     { Scroll_to_top();
       $('#idAlertConnexionLost').show();
       console.log("Error au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onclose = function (event)
     { Scroll_to_top();
       $('#idAlertConnexionLost').show();
       console.log("Close au websocket !");
       console.debug(event);
     }
    WTDWebSocket.onmessage = function (event)
     { var Response = JSON.parse(event.data);                                               /* Pointe sur <synoptique a=1 ..> */
       if (!Synoptique) return;

            if (Response.zmq_tag == "DLS_HISTO" && Messages_loaded==true)
        { if (Response.syn_id == Synoptique.id)                                     /* S'agit-il d'un message de notre page ? */
           { $('#idTableMessages').DataTable().ajax.reload( null, false ); }
        }
       else if (Response.zmq_tag == "DLS_CADRAN")
        { Changer_etat_cadran ( Response ); }
       else if (Response.zmq_tag == "SET_SYN_VARS")
        { $.each ( Response.syn_vars, function (i, item) { Set_syn_vars ( item.id, item ); } ); }
       else if (Response.zmq_tag == "DLS_VISUEL")
        { Changer_etat_visuel ( Response ); }
       else if (Response.zmq_tag == "pulse")
        { }
       else console.log("zmq_tag: " + Response.zmq_tag + " not known");
     }
  }
/******************************************************************************************************************************/
/* Load_page: Appelé au chargement de la page                                                                                 */
/******************************************************************************************************************************/
 function Load_page ()
  { vars = window.location.pathname.split('/');
    console.log("Load_page " + vars[1]);

    Charger_page_synoptique (1);
    Load_websocket();
    Ping();
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
