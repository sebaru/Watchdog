<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/onduleur.jpg" style="width:80px" alt="Onduleur 5PX">Liste des Onduleurs</h3></div>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="UPS_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="UPS_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>
    <table id="idTableUPS" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idUPSEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idUPSTitre"></span></h5>
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
           <label class="col-5 col-sm-4 col-form-label text-right">UPS Tech_ID</label>
           <input id="idUPSTechID" type="text" class="form-control" maxlength="32" placeholder="Tech_ID de l'UPS">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">UPS Name</label>
           <input id="idUPSName" required type="text" class="form-control" maxlength="32" placeholder="Nom de l'onduleur">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">UPS Host</label>
           <input id="idUPSHost" required type="text" class="form-control" placeholder="@IP ou hostname de l'onduleur">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Admin Username</label>
           <input id="idUPSAdminUsername" type="text" class="form-control" placeholder="Username de connexion à l'UPS">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Admin Password</label>
           <input id="idUPSAdminPassword" type="text" class="form-control" placeholder="Password de connexion à l'UPS">
          </div>
        </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idUPSValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/ups.js" type="text/javascript"></script>
