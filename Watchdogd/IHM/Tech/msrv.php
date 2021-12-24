<div class="container-fluid">

 <div class="row m-2">
   <div class="col-auto"><h3><i class="fas fa-crown text-danger"></i> Maintenance des Instances</h3> </div>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="MSRV_Refresh()" class="btn btn-outline-secondary"><i class="fas fa-redo"></i> Refresh</button>
   </div>
 </div>

<hr>

   <div class="table-responsive">
     <table id="idTableMSRV" class="table table-striped table-bordered table-hover">
       <thead class="thead-dark">
       </thead>
       <tbody>
       </tbody>
     </table>
   </div>

   <div class="alert alert-secondary">Configuration transverse</div>

    <div class="col form-group">
       <div class="input-group">
        <label class="col-5 col-sm-4 col-form-label text-right">Recharger la base d'icones</label>
        <button type="button" onclick="MSRV_Reload_icons()" class="btn btn-primary"><i class="fas fa-image"></i> Recharger les icones</button>
       </div>
    </div>


<!-- Container -->
</div>

<script src="/js/tech/msrv.js" type="text/javascript"></script>
