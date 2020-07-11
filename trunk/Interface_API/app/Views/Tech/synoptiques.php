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
    <div class="modal-content">
      <div class="modal-header bg-info">
        <h5 class="modal-title text-justify"><i class="fas fa-wrench"></i>Editer</h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalSynDetail">test</p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal">Annuler</button>
        <button type="button" class="btn btn-primary" data-dismiss="modal">Valider</button>
      </div>
    </div>
  </div>
</div>
