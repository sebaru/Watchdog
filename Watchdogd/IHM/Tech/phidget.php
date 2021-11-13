<div class="container-fluid">

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Le Thread <a href="/tech/process">Phidget</a> ne tourne pas !
   </div>

 <div class="row m-2">
   <h3><img src="/img/phidget_hub5000.jpg" style="width:80px" alt="Phidget HB5000">Liste des HUB5000 Phidgets</h3>
   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Phidget_Hub_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
        <button type="button" onclick="Redirect('/tech/phidget_map')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="Phidget_Hub_refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

    <table id="idTablePhidgetHub" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>


<div id="idModalPhidgetHub" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalPhidgetHubTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Tech_ID</label>
						     <input id="idModalPhidgetHubTechID" required type="text" class="form-control" maxlength="32" placeholder="Tech_ID du Hub">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Hostname</label>
						     <input id="idModalPhidgetHubHostname" required type="text" class="form-control" maxlength="32" placeholder="@IP ou hostname">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Mot de passe</label>
						     <input id="idModalPhidgetHubPassword" required type="text" class="form-control" placeholder="Mot de passe de connexion">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Serial Number</label>
						     <input id="idModalPhidgetHubSerial" required type="number" class="form-control" placeholder="Serial Number">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
						     <input id="idModalPhidgetHubDescription" type="text" class="form-control" placeholder="Ou est le hub ?">
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalPhidgetHubValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/phidget.js" type="text/javascript"></script>
