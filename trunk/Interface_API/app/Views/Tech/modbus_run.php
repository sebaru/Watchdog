<div class="container-fluid">

 <div class="row m-2">
 <h3>Etat des Modules WAGO sur Modbus</h3>

   <div class ="ml-auto btn-group">
        <button type="button" onclick="window.location='/tech/modbus_map'"class="btn btn-primary"><i class="fas fa-directions"></i> Map</button>
        <button type="button" onclick="window.location='/tech/modbus'" class="btn btn-secondary"><i class="fas fa-list"></i> Retour Liste</button>
   </div>
 </div>

   <div id="idAlertThreadNotRunning" class="alert alert-warning" role="alert" style="display: none">
     <h4 class="alert-heading">Warning !</h4>
         Thread "Modbus" is not running !
   </div>

    <table id="idTableModbusRun" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>
<script src="<?php echo base_url('js/modbus_run.js')?>" type="text/javascript"></script>
