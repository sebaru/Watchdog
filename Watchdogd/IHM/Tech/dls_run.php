<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-eye text-primary"></i> Etat du module '<strong id="idTitle"></strong>'</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Go_to_mnemos()" class="btn btn-primary"><i class="fas fa-book"></i> Mnemos</button>
        <button type="button" onclick="Go_to_dls_source()" class="btn btn-primary"><i class="fas fa-code"></i> Source</button>
        <button type="button" onclick="Redirect('/tech/dls')" class="btn btn-secondary"><i class="fas fa-list"></i> Liste DLS</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
</div>
<hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="/img/entree.png" /> Entrées TOR</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableEntreeTOR')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>

      <table id="idTableEntreeTOR" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>
<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="/img/entree_analogique.png" /> Entrées ANA</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableEntreeANA')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>

      <table id="idTableEntreeANA" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>
<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="/img/sortie.png" /> Sorties TOR</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableSortieTOR')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableSortieTOR" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="/img/sortie_analogique.png" /> Sorties ANA</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableSortieANA')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableSortieANA" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Registres"
                       src="/img/calculatrice.png" /> Registres</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableRegistre')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableRegistre" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Compteurs d'impulsions"
                       src="/img/front_montant.png" /> Compteurs d'impulsions</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableCI')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableCI" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Compteurs horaires"
                       src="/img/compteur_horaire.png" /> Compteurs horaires</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableCH')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableCH" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Temporisations"
                       src="/img/sablier.png" /> Temporisations</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableTempo')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableTempo" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Watchdogs"
                       src="/img/countdown.svg" /> Watchdogs</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableWatchdog')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableWatchdog" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Messages"
                       src="/img/message.png" /> Messages</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableMessages')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableMessages" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
<div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Monostables"
                       src="/img/message.png" /> Monostables</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableMONO')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableMONO" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
<div class="row m-2">
      <h4><img style="width: 30px" data-toggle="tooltip" title="Bistables"
                       src="/img/message.png" /> Bistables</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableBI')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableBI" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>

<!----------------------------------------------------------------------------------------------------------------------------->
<div class="row m-2">
      <h4><span><i class="fas fa-eye text-primary"></i></span> Visuels</h4>
     <div class="ml-auto">
        <button type="button" onclick="Dls_run_refresh('idTableVisuel')" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
			  </div>
</div>
      <table id="idTableVisuel" class="table table-striped table-bordered table-hover">
        <thead class="thead-dark">
        </thead>
        <tbody>
        </tbody>
      </table>
      <hr>
<!-- Container -->
</div>

<script src="/js/tech/dls_run.js" type="text/javascript"></script>
