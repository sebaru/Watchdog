<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-book text-primary"></i> Liste des Mnémoniques - '<strong id="idTitle"></strong>'</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Go_to_dls_run()"class="btn btn-info"><i class="fas fa-eye"></i> RUN</button>
        <button type="button" onclick="Go_to_dls_source()" class="btn btn-primary"><i class="fas fa-code"></i> Source</button>
        <button type="button" onclick="Redirect('/tech/dls')" class="btn btn-secondary"><i class="fas fa-list"></i> Retour</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>
          <ul class="nav nav-tabs" role="tablist">
            <li class="nav-item"><a class="nav-link active" data-toggle="tab" href="#idTabEntreeTor">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Entrées TOR"
                       src="/img/entree.png" />Entrées TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabEntreeAna">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Entrées ANA"
                       src="/img/entree_analogique.png" />Entrées ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieTor">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Sorties TOR"
                       src="/img/sortie.png" />Sorties TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieAna">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Sorties ANA"
                       src="/img/sortie_analogique.png" />Sorties ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabRegistre">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Registres"
                       src="/img/calculatrice.png" />Registres</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabCptImp">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Compteurs d'impulsion"
                       src="/img/front_montant.png" />Compteurs d'impulsion</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabCptH">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Compteurs horaire"
                       src="/img/compteur_horaire.png" />Compteurs horaire</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabTempo">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Temporisations"
                       src="/img/sablier.png" />Tempos</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabHorloge">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Horloges"
                       src="/img/bit_horloge.png" /> Horloges</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabWatchdog">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Watchdogs"
                       src="/img/countdown.svg" /> Watchdogs</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabMessage">
                  <img class="wtd-img-bit-interne" data-toggle="tooltip" title="Messages"
                       src="/img/message.png" /> Messages</a></li>
          </ul>

<div class="tab-content">
  <div id="idTabEntreeTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableEntreeTor" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabSortieTor" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableSortieTor" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabEntreeAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableEntreeAna" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabSortieAna" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableSortieAna" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabCptH" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableCptH" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabCptImp" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableCptImp" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabRegistre" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableRegistre" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabTempo" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableTempo" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabWatchdog" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableWatchdog" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabMessage" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableMessage" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

  <div id="idTabHorloge" class="tab-pane fade in table-responsive mt-1" role="tabpanel">
    <table id="idTableHorloge" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>
  </div>

</div> <!-- tab content -->

<!-- Container -->
</div>

<script src="/js/tech/common_tech.js" type="text/javascript"></script>
<script src="/js/tech/mnemos.js" type="text/javascript"></script>
