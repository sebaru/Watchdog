<div class="container">

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread <a href="/tech/process">GPIOD</a> is not running !
   </div>

 <div class="row m-2">
   <h3><img src="/img/raspberrypi.png" style="width:80px" alt="Configuration GPIOD"> Configuration Gpio/Raspberry</h3>

   <div class ="ml-auto btn-group align-items-center">
        <button type="button" onclick="Redirect('/tech/command_text')" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="GPIOD_Load_config()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
        <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Choix de l'instance</label>
           <select id="idTargetProcess" class="custom-select border-info" onchange="GPIOD_Load_config()"></select>
          </div>
       </div>

    <table id="idGPIODTable" class="table table-striped table-bordered table-hover w-100">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idModalGPIOD" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalGPIODTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Mode Input/Output</label>
           <select id="idModalGPIODInOut" class="col-7 custom-select border-info">
             <option value="0">Input</option>
             <option value="1">Output</option>
           </select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Mode Active Low</label>
           <select id="idModalGPIODActiveLow" class="col-7 custom-select border-info">
             <option value="0">Active High</option>
             <option value="1">Active Low</option>
           </select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Rechercher une Target</label>
           <input id="idModalGPIODRechercherTechID" type="text" class="col-7 form-control" placeholder="Rechercher un Tech_id">
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target TechID</label>
           <select id="idModalGPIODSelectTechID" class="col-7 custom-select border-info"></select>
          </div>
       </div>

       <div class="col form-group">
          <div class="input-group">
           <label class="col-5 col-sm-4 col-form-label text-right">Target Acronyme</label>
           <select id="idModalGPIODSelectAcronyme" class="col-7 custom-select border-info"></select>
          </div>
       </div>


      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalGPIODValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>


<script src="/js/tech/gpiod.js" type="text/javascript"></script>
