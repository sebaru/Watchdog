<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/sms.jpg" style="width:80px" alt="Configuration SMS">Configuration des SMSG</h3></div>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="SMSG_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un GSM</button>
        <button type="button" onclick="SMSG_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

   <div class="table-responsive">
     <table id="idTableSMSG" class="table table-striped table-bordered table-hover">
       <thead class="thead-dark">
       </thead>
       <tbody>
       </tbody>
     </table>
   </div>

<!-- Container -->
</div>


<div id="idSMSGEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idSMSGTitre"></span></h5>
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
           <label class="col-5 col-sm-4 col-form-label text-right">SMSG Tech_ID</label>
           <input id="idSMSGTechID" type="text" class="form-control" maxlength="32" placeholder="Tech_ID du SMS">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">SMSG Description</label>
           <input id="idSMSGDescription" type="text" class="form-control" placeholder="Description du téléphone et/ou sa position">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">OVH Service Name</label>
           <input id="idSMSGOVHServiceName" type="text" class="form-control" placeholder="OVH Service Name (sms-xxxxxx-1)">
           <a target="_blank" href="https://eu.api.ovh.com/createToken/index.cgi?GET=/sms&GET=/sms/*/jobs&POST=/sms/*/jobs" class="col-2 col-form-label">Creer un token</a>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">OVH Application Key</label>
           <input id="idSMSGOVHApplicationKey" type="text" class="form-control" placeholder="OVH Application Key">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">OVH Application Secret</label>
           <input id="idSMSGOVHApplicationSecret" type="text" class="form-control" placeholder="OVH Application Secret">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">OVH Consumer Key</label>
           <input id="idSMSGOVHConsumerKey" type="text" class="form-control" placeholder="OVH Consumer Key">
          </div>
       </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idSMSGValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/smsg.js" type="text/javascript"></script>
