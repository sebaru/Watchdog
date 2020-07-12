<div class="container-fluid">

<div class="table-responsive-lg">
  <table id="idTableSyn" class="table table-striped table-bordered table-hover">
    <thead class="thead-dark">
				</thead>
			 <tbody>
    </tbody>
  </table>
</div>
<script src="<?php echo base_url('js/synoptiques.js')?>" type="text/javascript"></script>
<!-- Container -->
</div>

<div id="idModalSynEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-secondary text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-wrench"></i> <span id="idModalSynEditTitre">Editer</span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <div class="row form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text"><i class="fas fa-star"></i></span>
						     </div>
						     <input id="idModalSynEditAccessLevel" type="text" class="form-control" placeholder="Access_level">
     					</div>
        </div>
        <div class="row form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text"><i class="fas fa-pen"></i></span>
						     </div>
						     <input id="idModalSynEditDescription" type="text" class="form-control" placeholder="Description">
     					</div>
        </div>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal">Annuler</button>
        <button type="button" class="btn btn-primary" data-dismiss="modal">Valider</button>
      </div>
    </div>
  </div>
</div>
