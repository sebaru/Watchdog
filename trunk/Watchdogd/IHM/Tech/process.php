<div class="container-fluid">

 <div class="row m-2 inline">
   <div class="col-auto"> <h3><i class="fas fa-microchip text-primary"></i> Liste des Processus sur</h3> </div>
  	<div class="col-auto mr-auto">
     <select id='idTargetInstance' onchange="Process_change_target_instance()" class="custom-select border-info d.inline"></select>
   </div>
 </div>

<hr>

   <div class="table-responsive">
     <table id="idTableProcess" class="table table-striped table-bordered table-hover">
       <thead class="thead-dark">
				   </thead>
			    <tbody>
       </tbody>
     </table>
   </div>

<script src="/js/tech/process.js" type="text/javascript"></script>
<!-- Container -->
</div>
