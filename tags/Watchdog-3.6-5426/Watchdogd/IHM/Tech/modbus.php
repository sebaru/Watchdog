<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/wago_750342.webp" style="width:80px" alt="Wago 750-342">Liste des Modules WAGO sur Modbus</h3>
   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Modbus_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
        <button type="button" onclick="window.location='/tech/modbus_map'" class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="Modbus_refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Le Thread <a href="/tech/process">Modbus</a> ne tourne pas !
   </div>

    <table id="idTableModbus" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<hr>
 <h3>Détails des Modules WAGO sur Modbus (RUN)</h3>

    <table id="idTableModbusRun" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>


<div id="idModalModbusEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalModbusEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">TechID</label>
						     <input id="idModalModbusEditTechID" required type="text" class="form-control" placeholder="Tech ID du module">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Hostname</label>
						     <input id="idModalModbusEditHostname" required type="text" class="form-control" placeholder="@IP ou hostname">
     					</div>
   					</div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description</label>
						     <input id="idModalModbusEditDescription" type="text" class="form-control" placeholder="Ou est le module ?">
     					</div>
        </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Watchdog</label>
						     <input id="idModalModbusEditWatchdog" type="number" class="form-control" min=10 max=1200 placeholder="Nombre de 1/10 de secondes avant de couper les sorties">
						     <div class="input-group-append">
							     <span class="input-group-text">secondes</span>
						     </div>
     					</div>
        </div>

       <div class="col form-group">
					     <div class="input-group align-items-center ">
						     <label class="col-5 col-sm-4 col-form-label text-right">Max Requetes/sec</label>
						     <input id="idModalModbusEditMaxRequestParSec" type="number" class="form-control" min=1 max=100 placeholder="Nombre de requetes max par seconde">
						     <div class="input-group-append">
							     <span class="input-group-text form-control">par seconde</span>
						     </div>
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalModbusEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<script src="/js/tech/modbus.js" type="text/javascript"></script>
