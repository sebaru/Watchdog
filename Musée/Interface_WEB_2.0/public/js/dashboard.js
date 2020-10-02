 document.addEventListener('DOMContentLoaded', Load_dashboard, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_dashboard ()
  { console.log ("in load dashboard !");

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/status", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { Show_Error ( xhr.statusText );
          return;
        }
       var Response = JSON.parse(xhr.responseText);
       console.debug(Response);
       document.getElementById("idNbrSyns").innerHTML = Response.nbr_syns;
       document.getElementById("idNbrSynsMotifs").innerHTML = Response.nbr_syns_motifs;
       document.getElementById("idNbrSynsLiens").innerHTML = Response.nbr_syns_liens;
       document.getElementById("idNbrDls").innerHTML = Response.nbr_dls;
       document.getElementById("idDlsBitparsec").innerHTML = Response.bit_par_sec;
       document.getElementById("idDlsTourparsec").innerHTML = Response.tour_par_sec;
       document.getElementById("idNbrDlsLignes").innerHTML = Response.nbr_dls_lignes;
       document.getElementById("idNbrDlsDI").innerHTML = Response.nbr_dls_di;
       document.getElementById("idNbrDlsDO").innerHTML = Response.nbr_dls_do;
       document.getElementById("idNbrDlsAI").innerHTML = Response.nbr_dls_ai;
       document.getElementById("idNbrDlsAO").innerHTML = Response.nbr_dls_ao;
       document.getElementById("idNbrDlsBOOL").innerHTML = Response.nbr_dls_bool;
       document.getElementById("idNbrUsers").innerHTML = Response.nbr_users;
       document.getElementById("idNbrAuditLog").innerHTML = Response.nbr_audit_log;
       document.getElementById("idNbrSessions").innerHTML = Response.nbr_sessions;
       document.getElementById("idNbrMsgs").innerHTML = Response.nbr_msgs;
       document.getElementById("idNbrHistoMsgs").innerHTML = Response.nbr_histo_msgs;
       document.getElementById("idArchDBUsername").innerHTML = Response.archdb_username;
       document.getElementById("idArchDBHostname").innerHTML = Response.archdb_hostname;
       document.getElementById("idArchDBPort").innerHTML = Response.archdb_port;
       document.getElementById("idArchDBDatabase").innerHTML = Response.archdb_database;
       document.getElementById("idDBUsername").innerHTML = Response.db_username;
       document.getElementById("idDBHostname").innerHTML = Response.db_hostname;
       document.getElementById("idDBPort").innerHTML = Response.db_port;
       document.getElementById("idDBDatabase").innerHTML = Response.db_database;

       document.getElementById("idConfigVersion").innerHTML = Response.version;
       document.getElementById("idConfigRunAs").innerHTML = Response.run_as;
       document.getElementById("idConfigStarted").innerHTML = Response.started;
       document.getElementById("idConfigTop").innerHTML = Response.top;
     };
    xhr.send();
    document.getElementById("idUsername").innerHTML = sessionStorage.getItem("username");
    Charger_une_courbe ( "idCourbeDlsTourParSec", "SYS", "DLS_TOUR_PER_SEC", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsBitParSec", "SYS", "DLS_BIT_PER_SEC", "HOUR" );
    Charger_une_courbe ( "idCourbeDlsAttente", "SYS", "DLS_WAIT", "HOUR" );
    Charger_une_courbe ( "idCourbeNbArchive", "SYS", "ARCH_REQUEST_NUMBER", "HOUR" );
  }
