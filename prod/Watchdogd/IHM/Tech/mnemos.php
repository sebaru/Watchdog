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
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="/img/entree.png" />Entrées TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabEntreeAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="/img/entree_analogique.png" />Entrées ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieTor">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="/img/sortie.png" />Sorties TOR</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabSortieAna">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="/img/sortie_analogique.png" />Sorties ANA</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabRegistre">
                  <img style="width: 30px" data-toggle="tooltip" title="Registres"
                       src="/img/calculatrice.png" />Registres</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabCptImp">
                  <img style="width: 30px" data-toggle="tooltip" title="Compteurs d'impulsion"
                       src="/img/front_montant.png" />Compteurs d'impulsion</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabCptH">
                  <img style="width: 30px" data-toggle="tooltip" title="Compteurs horaire"
                       src="/img/compteur_horaire.png" />Compteurs horaire</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabTempo">
                  <img style="width: 30px" data-toggle="tooltip" title="Temporisations"
                       src="/img/sablier.png" />Tempos</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabHorloge">
                  <img style="width: 30px" data-toggle="tooltip" title="Horloges"
                       src="/img/calendar.svg" /> Horloges</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabWatchdog">
                  <img style="width: 30px" data-toggle="tooltip" title="Watchdogs"
                       src="/img/countdown.svg" /> Watchdogs</a></li>
            <li class="nav-item"><a class="nav-link" data-toggle="tab" href="#idTabMessage">
                  <img style="width: 30px" data-toggle="tooltip" title="Messages"
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


<div id="idModalSynEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalSynEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Parent</span>
						     </div>
						     <input id="idModalSynEditPPage" type="text" class="form-control" placeholder="Parent du synoptique">
     					</div>
        </div>

        <div class="form-row form-group">

					     <div class="col-7 input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Page</span>
						     </div>
						     <input id="idModalSynEditPage" type="text" class="form-control" placeholder="Titre du synoptique">
     					</div>

					     <div class="col-5 input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text"><i class="fas fa-star"></i></span>
						     </div>
						     <input id="idModalSynEditAccessLevel" type="number" class="form-control" min=0 max=9 placeholder="Level">
     					</div>

        </div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Description</span>
						     </div>
						     <input id="idModalSynEditDescription" type="text" class="form-control" placeholder="Description du synoptique">
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalSynEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalSynDel" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-danger text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-trash"></i> <span id="idModalSynDelTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalSynDelMessage">Une erreur est survenue !</p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalSynDelValider" type="button" class="btn btn-danger" data-dismiss="modal"><i class="fas fa-trash"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/common_tech.js" type="text/javascript"></script>
<script src="/js/tech/mnemos.js" type="text/javascript"></script>
