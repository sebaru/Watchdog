<div class="container-fluid">

 <div class="row m-2">
 <h3>Paramétrage des Archives</h3>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="Archive_sauver_parametre()" class="btn btn-outline-success"><i class="fas fa-save"></i> Sauvegarder</button>
        <button type="button" onclick="Archive_testdb()" class="btn btn-outline-info"><i class="fas fa-question"></i> TestDB</button>
        <button type="button" onclick="Archive_purge()" class="btn btn-outline-warning"><i class="fas fa-history"></i> Purge</button>
        <button type="button" onclick="Archive_clear()" class="btn btn-outline-danger"><i class="fas fa-eraser"></i> Clear</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-list"></i> Retour Liste</button>
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Le Thread "Archive" ne tourne pas !
   </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Database Server Hostname</label>
						     <input id="idArchiveDBHostname" type="text" class="form-control" placeholder="ArchiveDB Server">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Database Port</label>
						     <input id="idArchiveDBPort" type="number" min="0" max="65535" class="form-control" placeholder="ArchiveDB Port">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Database Name</label>
						     <input id="idArchiveDBDatabase" type="text" class="form-control" placeholder="ArchiveDB Name">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Database Username</label>
						     <input id="idArchiveDBUsername" type="text" class="form-control" placeholder="ArchiveDB Username">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Database Password</label>
						     <input id="idArchiveDBPassword" type="password" class="form-control" placeholder="Lassez vide pour ne pas changer de password">
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Durée de retention</label>
						     <input id="idArchiveDBRetention" type="number" min="1" class="form-control" placeholder="Nombre de jours">
						     <div class="input-group-append">
							     <span class="input-group-text">jours</span>
						     </div>
     					</div>
  					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-3 col-form-label text-right">Taille du tampon</label>
						     <input id="idArchiveDBBufferSize" type="number" min="100" class="form-control" placeholder="Nombre d'enregistrements">
						     <div class="input-group-append">
							     <span class="input-group-text">enregistrements</span>
						     </div>
     					</div>
  					</div>

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
