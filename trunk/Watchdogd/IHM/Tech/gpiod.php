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
           <select id="idTargetInstance" class="custom-select border-info" onchange="GPIOD_Load_config()"></select>
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

<script src="/js/tech/gpiod.js" type="text/javascript"></script>
