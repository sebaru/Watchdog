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

   <table id="idTableConfArchive" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<hr>
 <h3>Gestion des tables d'archivage</h3>

    <table id="idTableArchive" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<script src="<?php echo base_url('js/tech/archive.js')?>" type="text/javascript"></script>
