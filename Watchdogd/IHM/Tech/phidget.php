<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/phidget_hub5000.jpg" style="width:80px" alt="Phidget HB5000">Liste des HUB5000 Phidgets</h3></div>
   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="PHIDGET_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="PHIDGET_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

    <table id="idTablePHIDGET" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>


<div id="idPHIDGETEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idPHIDGETTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>

      <div class="modal-body">
       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Choix de l'instance</label>
           <select id="idTargetProcess" class="custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">TechID</label>
           <input id="idPHIDGETTechID" required type="text" class="form-control" placeholder="Tech ID du module">
          </div>
        </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
						     <input id="idPHIDGETDescription" type="text" class="form-control" placeholder="Ou est le hub ?">
     					</div>
        </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Hostname</label>
						     <input id="idPHIDGETHostname" required type="text" class="form-control" maxlength="32" placeholder="@IP ou hostname">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Mot de passe</label>
						     <input id="idPHIDGETPassword" required type="text" class="form-control" placeholder="Mot de passe de connexion">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Serial Number</label>
						     <input id="idPHIDGETSerial" required type="number" class="form-control" placeholder="Serial Number">
     					</div>
   					</div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idPHIDGETValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/phidget.js" type="text/javascript"></script>
