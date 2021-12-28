 document.addEventListener('DOMContentLoaded', Load_dashboard, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_dashboard ()
  { console.log ("in load dashboard !");

    Send_to_API ( "GET", "/api/status", null, function (Response)
     { console.debug(Response);
       document.getElementById("idNbrSyns").innerHTML = Response.nbr_syns;
       document.getElementById("idNbrSynsVisuels").innerHTML = Response.nbr_syns_visuels;
       document.getElementById("idNbrSynsLiens").innerHTML = Response.nbr_syns_liens;
       document.getElementById("idNbrDls").innerHTML = Response.nbr_dls;
       document.getElementById("idDlsBitparsec").innerHTML = Response.bit_par_sec;
       document.getElementById("idDlsTourparsec").innerHTML = Response.tour_par_sec;
       document.getElementById("idNbrDlsLignes").innerHTML = Response.nbr_dls_lignes;
       document.getElementById("idNbrDlsDI").innerHTML = Response.nbr_dls_di;
       document.getElementById("idNbrDlsDO").innerHTML = Response.nbr_dls_do;
       document.getElementById("idNbrDlsAI").innerHTML = Response.nbr_dls_ai;
       document.getElementById("idNbrDlsAO").innerHTML = Response.nbr_dls_ao;
       document.getElementById("idNbrDlsBI").innerHTML = Response.nbr_dls_bi;
       document.getElementById("idNbrDlsMONO").innerHTML = Response.nbr_dls_mono;
       document.getElementById("idNbrUsers").innerHTML = Response.nbr_users;
       document.getElementById("idNbrAuditLog").innerHTML = Response.nbr_audit_log;
       document.getElementById("idNbrSessions").innerHTML = Response.nbr_sessions;
       document.getElementById("idNbrMsgs").innerHTML = Response.nbr_msgs;
       document.getElementById("idNbrHistoMsgs").innerHTML = Response.nbr_histo_msgs;
       document.getElementById("idArchDBUsername").innerHTML = Response.archdb_username;
       document.getElementById("idArchDBHostname").innerHTML = Response.archdb_hostname;
       document.getElementById("idArchDBPort").innerHTML = Response.archdb_port;
       document.getElementById("idArchDBDatabase").innerHTML = Response.archdb_database;
       $("#idArchDBNbrEnreg").text(Response.archdb_nbr_enreg);
       if (Response.archdb_nbr_enreg>100) $("#idArchDBNbrEnreg").addClass("text-danger");
                                     else $("#idArchDBNbrEnreg").removeClass("text-danger");
       document.getElementById("idDBUsername").innerHTML = Response.db_username;
       document.getElementById("idDBHostname").innerHTML = Response.db_hostname;
       document.getElementById("idDBPort").innerHTML = Response.db_port;
       document.getElementById("idDBDatabase").innerHTML = Response.db_database;

       document.getElementById("idConfigInstance").innerHTML = Response.instance;
       document.getElementById("idConfigMaster").innerHTML = (Response.instance_is_master ? "TRUE" : "FALSE");
       document.getElementById("idConfigMasterHost").innerHTML = Response.master_host;
       document.getElementById("idConfigVersion").innerHTML = Response.version;
       document.getElementById("idConfigRunAs").innerHTML = Response.run_as;
       document.getElementById("idConfigStarted").innerHTML = Response.started;
       document.getElementById("idConfigTop").innerHTML = Response.top;
     });
  }
