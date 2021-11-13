	$(document).ready(function() {

		$("#msgfdl").DataTable( {
			"pageLength" : 100,
			/*"ajax": {
				url : base_url + "admin/dls/get",
				type : 'GET'
			},
   "order": [ [5, "asc"], [6, "asc"] ],*/
			responsive: true,
   /*fixedHeader: true,*/
		});


	});


/******************************************************************************************************************************/
 function Type_to_icone(type)
  { if (typeof type == 'string') type = parseInt(type);
    switch(type)
     { case 0: return('<img style="width: 20px" data-toggle="tooltip" title="Information" src="https://icons.abls-habitat.fr/assets/gif/Pignon_vert.svg" /> Info');
       case 2: return('<img style="width: 20px" data-toggle="tooltip" title="Défaut"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_jaune.svg" /> Défaut');
       case 3: return('<img style="width: 20px" data-toggle="tooltip" title="Alarme"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_orange.svg" /> Alarme');
       case 4: return('<img style="width: 20px" data-toggle="tooltip" title="Veille"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_vert.svg" /> Veille');
       case 1: return('<img style="width: 20px" data-toggle="tooltip" title="Alerte"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_rouge.svg" /> Alerte');
       case 5: return('<img style="width: 20px" data-toggle="tooltip" title="Attente"     src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_noir.svg" /> Attente');
       case 6: return('<img style="width: 20px" data-toggle="tooltip" title="Danger"      src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_rouge.svg" /> Danger');
       case 7: return('<img style="width: 20px" data-toggle="tooltip" title="Dérangement" src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_jaune.svg" /> Dérangement');
     }
  }
/******************************************************************************************************************************/
 function Type_to_color(type)
  { if (typeof type == 'string') type = parseInt(type);
      switch (type)
     { case 0: return("info");
       case 4: return("success");
       case 5: return("info");
       case 3:
       case 6:
       case 1: return("danger");
       case 7:
       case 2: return("warning");
     }
  }
 function acquitter_msg ( tech_id, acronyme )
  {var xhr = new XMLHttpRequest;
    xhr.open('get',base_url + "msgs/acquit/" + tech_id + "/" + acronyme, true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       console.log ("Message "+tech_id + "/" + acronyme+" bien acquitté");
     };
    xhr.send();
  }
 function Handle_msgs ( Response )
  { table = $('#msgfdl').DataTable();
    if (Response.alive === false)
     { row = document.getElementById("#MSGFDL-"+Response.tech_id+"-"+Response.acronyme);
       table.row(row).remove().draw();
     }
     else
     { row = document.getElementById("#MSGFDL-"+Response.tech_id+"-"+Response.acronyme);
       if (row)
        { if (Response.hasOwnProperty("date_create"))
           { table.cell(row, 0).data( Response.date_create ); }
          if (Response.hasOwnProperty("nom_ack") && Response.nom_ack != null)
           { table.cell(row, 4).data( Response.nom_ack ); }
          if (Response.hasOwnProperty("date_fixe"))
           { table.cell(row, 5).data( Response.date_fixe ); };
        }
       else
        { table.row.add( [ Response.date_create, Type_to_icone(Response.type),
                           Response.dls_shortname, Response.libelle, Response.nom_ack, Response.date_fixe,
                           "<button onclick=acquitter_msg('"+Response.tech_id+"','"+Response.acronyme+"') class='btn btn-"+Type_to_color(Response.type)+"'>Acquitter</button>"
                        /*   "<a href='msgs/acquit/"+Response.id+"'><span class='label label-"+Type_to_color(Response.type)+"'>Acquitter</span>"*/
                         ]
                       ).node().id="#MSGFDL-"+Response.tech_id+"-"+Response.acronyme;
        }
       table.draw();
     }
  }
/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Charger_ws_msgs ()
  { var msgfdl = document.getElementById("msgfdl");
    if (!msgfdl) return;

    var xhr = new XMLHttpRequest;
    xhr.open('get',base_url + "api/histo/alive", true);
    xhr.onreadystatechange = function()
     { if ( ! (xhr.readyState == 4 && (xhr.status == 200 || xhr.status == 0)) ) return;
       var Response = JSON.parse(xhr.responseText);                                         /* Pointe sur <synoptique a=1 ..> */
       Response.enregs.forEach ( msg => Handle_msgs (msg) );
     };
    xhr.send();

    if (window.location.hostname=="test.watchdog.fr")
         var WTDWebSocket = new WebSocket("ws://test.watchdog.fr/ws/live-msgs", "live-msgs");
    else var WTDWebSocket = new WebSocket("wss://"+window.location.hostname+":"+window.location.port+"/ws/live-msgs", "live-msgs");
    WTDWebSocket.onopen = function (event)
     { console.log("Connecté au websocket !");
       /*this.send ( JSON.stringify( { syn_id: id } ) );*/
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
     { //console.log("Recu MSGS "+event.data);
       var Response = JSON.parse(event.data);                                               /* Pointe sur <synoptique a=1 ..> */
       if (Response.zmq_type == "update_histo")
        { Handle_msgs(Response); }
       else if (Response.zmq_type == "pulse")
        { console.log("Pulse reçu !"); }
     }
  }
