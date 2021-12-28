<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/linky.jpg" style="width:80px" alt="Teleinfo E.D.F"> Configuration des modules Téléinfo EDF</h3></div>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="TELEINFO_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="TELEINFO_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>


   <div class="table-responsive">
     <table id="idTableTELEINFO" class="table table-striped table-bordered table-hover">
       <thead class="thead-dark">
       </thead>
       <tbody>
       </tbody>
     </table>
   </div>

<!-- Container -->
</div>


<div id="idTELEINFOEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idTELEINFOTitre"></span></h5>
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
           <label class="col-5 col-sm-4 col-form-label text-right">TELEINFO Tech_ID</label>
           <input id="idTELEINFOTechID" type="text" class="form-control" maxlength="32" placeholder="Tech_ID du TELEINFO">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">TELEINFO Description</label>
           <input id="idTELEINFODescription" type="text" class="form-control" placeholder="Description du téléphone et/ou sa position">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Port Device</label>
           <input id="idTELEINFOPort" type="text" class="form-control" placeholder="FileSystem device /dev/xxx">
          </div>
       </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idTELEINFOValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/teleinfoedf.js" type="text/javascript"></script>
