<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><img src="/img/wago_750342.webp" style="width:80px" alt="Wago 750-342">Liste des Modules WAGO sur Modbus</h3></div>
   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="MODBUS_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter une connexion</button>
        <button type="button" onclick="MODBUS_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

    <table id="idTableMODBUS" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idMODBUSEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idMODBUSTitre"></span></h5>
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
           <input id="idMODBUSTechID" required type="text" class="form-control" placeholder="Tech ID du module">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
           <input id="idMODBUSDescription" type="text" class="form-control" placeholder="Ou est le module ?">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Hostname</label>
           <input id="idMODBUSHostname" required type="text" class="form-control" placeholder="@IP ou hostname">
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Watchdog</label>
           <input id="idMODBUSWatchdog" type="number" class="form-control" min=10 max=1200 placeholder="Nombre de 1/10 de secondes avant de couper les sorties">
           <div class="input-group-append">
            <span class="input-group-text">secondes</span>
           </div>
          </div>
        </div>

       <div class="col form-group">
          <div class="input-group align-items-center ">
           <label class="col-5 col-sm-4 col-form-label text-right">Max Requetes/sec</label>
           <input id="idMODBUSMaxRequestParSec" type="number" class="form-control" min=1 max=100 placeholder="Nombre de requetes max par seconde">
           <div class="input-group-append">
            <span class="input-group-text form-control">par seconde</span>
           </div>
          </div>
        </div>

      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idMODBUSValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/modbus.js" type="text/javascript"></script>
