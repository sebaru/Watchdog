 document.addEventListener('DOMContentLoaded', Load_dashboard, false);

/********************************************* Chargement du synoptique 1 au démrrage *****************************************/
 function Load_dashboard ()
  { console.log ("in load dashboard !");

    var xhr = new XMLHttpRequest;
    xhr.open('GET', "/api/status", true);
    xhr.onreadystatechange = function()
     { if ( xhr.readyState != 4 ) return;
       if (xhr.status != 200)
        { document.getElementById("idModalDetail").innerHTML = "Une erreur inconnue est survenue.";
          $('#idModalError').modal("show");
        }
       var Response = JSON.parse(xhr.responseText);
       console.debug(Response);
       document.getElementById("idNbrSyns").innerHTML = Response.nbr_syns;
       document.getElementById("idNbrSynsMotifs").innerHTML = Response.nbr_syns_motifs;
       document.getElementById("idNbrSynsLiens").innerHTML = Response.nbr_syns_liens;
       document.getElementById("idNbrDls").innerHTML = Response.nbr_dls;
       document.getElementById("idNbrDlsLignes").innerHTML = Response.nbr_dls_lignes;
       document.getElementById("idNbrDlsDI").innerHTML = Response.nbr_dls_di;
       document.getElementById("idNbrDlsDO").innerHTML = Response.nbr_dls_do;
       document.getElementById("idNbrDlsAI").innerHTML = Response.nbr_dls_ai;
       document.getElementById("idNbrDlsAO").innerHTML = Response.nbr_dls_ao;
       document.getElementById("idNbrDlsBOOL").innerHTML = Response.nbr_dls_bool;
       document.getElementById("idNbrUsers").innerHTML = Response.nbr_users;
       document.getElementById("idNbrAuditLog").innerHTML = Response.nbr_audit_log;
       document.getElementById("idNbrMsgs").innerHTML = Response.nbr_msgs;
       document.getElementById("idNbrHistoMsgs").innerHTML = Response.nbr_histo_msgs;
     };
    xhr.send();

  }
