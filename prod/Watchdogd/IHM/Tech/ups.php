<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/onduleur.jpg" style="width:80px" alt="Onduleur 5PX">Liste des Onduleurs</h3>
   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Ups_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
        <button type="button" onclick="Ups_refresh" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <button type="button" onclick="Redirect('/tech/process')" class="btn btn-secondary"><i class="fas fa-microchip"></i> Processus</button>
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Le Thread <a href="/tech/process">Ups</a> ne tourne pas !
   </div>

    <table id="idTableUps" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<div id="idModalUpsEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalUpsEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">TechID</span>
						     </div>
						     <input id="idModalUpsEditTechID" type="text" class="form-control" placeholder="Tech ID du module">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Name</span>
						     </div>
						     <input id="idModalUpsEditName" type="text" class="form-control" placeholder="Nom de l'onduleur">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Host</span>
						     </div>
						     <input id="idModalUpsEditHost" type="text" class="form-control" placeholder="@IP ou hostname de l'onduleur">
     					</div>
   					</div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Admin Username</span>
						     </div>
						     <input id="idModalUpsEditAdminUsername" type="text" class="form-control" placeholder="Username de connexion à l'UPS">
     					</div>
        </div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Admin Password</span>
						     </div>
						     <input id="idModalUpsEditAdminPassword" type="text" class="form-control" placeholder="Password de connexion à l'UPS">
     					</div>
        </div>

<!--
        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Watchdog</span>
						     </div>
						     <input id="idModalUpsEditWatchdog" type="number" class="form-control" min=10 max=1200 placeholder="Nombre de 1/10 de secondes avant de couper les sorties">
						     <div class="input-group-append">
							     <span class="input-group-text">secondes</span>
						     </div>
     					</div>
        </div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Max Requetes/sec</span>
						     </div>
						     <input id="idModalUpsEditMaxRequestParSec" type="number" class="form-control" min=1 max=100 placeholder="Nombre de requetes max par seconde">
						     <div class="input-group-append">
							     <span class="input-group-text">par seconde</span>
						     </div>
     					</div>
        </div>
-->
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalUpsEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/ups.js" type="text/javascript"></script>
