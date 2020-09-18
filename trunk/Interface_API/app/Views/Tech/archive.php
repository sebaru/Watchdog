<div class="container">

 <div class="row m-2">
 <h3>Paramétrage des Archives</h3>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="Archive_testdb()" class="btn btn-outline-success"><i class="fas fa-question"></i> TestDB</button>
        <button type="button" onclick="Archive_purge()" class="btn btn-outline-warning"><i class="fas fa-history"></i> Purge</button>
        <button type="button" onclick="Archive_clear()" class="btn btn-outline-danger"><i class="fas fa-eraser"></i> Clear</button>
   </div>
 </div>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread "Archive" is not running !
   </div>

    <table id="idTableArchive" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idModalDel" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-danger text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-trash"></i> <span id="idModalDelTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">
        <p id="idModalDelMessage"></p>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalDelValider" type="button" class="btn btn-danger" data-dismiss="modal"><i class="fas fa-trash"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="<?php echo base_url('js/tech/archive.js')?>" type="text/javascript"></script>
