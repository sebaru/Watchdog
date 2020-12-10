<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-microchip text-primary"></i> Liste des Synoptiques</strong></h3>

   <div class ="ml-auto btn-group align-items-start">
   <!--     <button type="button" onclick="GSM_Sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
        <button type="button" onclick="GSM_test()" class="btn btn-outline-info"><i class="fas fa-question"></i> Test d'envoi</button>
        <button type="button" onclick="Redirect('/tech/smsg_map')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="GSM_Reload()" class="btn btn-outline-warning"><i class="fas fa-redo"></i> Reload</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

<div class="table-responsive-lg">
  <table id="idTableSyn" class="table table-striped table-bordered table-hover">
    <thead class="thead-dark">
				</thead>
			 <tbody>
    </tbody>
  </table>
</div>


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

<script src="/js/tech/synoptiques.js" type="text/javascript"></script>
