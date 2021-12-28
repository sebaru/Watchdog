<div class="container">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/imsgs.png" style="width:80px" alt="Configuration IMSGS">Configuration Messagerie Instantanée</h3></div>

   <div class="ml-auto btn-group align-items-center">
        <button type="button" onclick="IMSGS_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="IMSGS_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>
   <div class="table-responsive">
     <table id="idTableIMSGS" class="table table-striped table-bordered table-hover">
       <thead class="thead-dark">
       </thead>
       <tbody>
       </tbody>
     </table>
   </div>

<!-- Container -->
</div>

<div id="idIMSGSEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idIMSGSTitre"></span></h5>
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
           <label class="col-5 col-sm-4 col-form-label text-right">IMSG Tech_ID</label>
           <input id="idIMSGSTechID" type="text" class="form-control" maxlength="32" placeholder="Tech_ID du Thread">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
           <input id="idIMSGSDescription" type="text" class="form-control" placeholder="Description du module">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">XMPP JabberID</label>
           <div class="input-group-prepend">
             <div class="input-group-text">@</div>
           </div>
           <input id="idIMSGSJabberID" type="email" class="form-control" placeholder="username@server.tld">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">XMPP Password</label>
           <input id="idIMSGSPassword" type="password" class="form-control" placeholder="">
          </div>
       </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idIMSGSValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/imsgs.js" type="text/javascript"></script>
