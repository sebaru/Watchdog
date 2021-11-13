<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-code text-primary"></i> Edition D.L.S <strong id="idSourceTitle"></strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Compiler()" class="btn btn-outline-success"><i class="fas fa-coffee"></i> Compiler</button>
        <button type="button" onclick="Go_to_dls_run()"class="btn btn-info"><i class="fas fa-eye"></i> RUN</button>
        <button type="button" onclick="Go_to_mnemos()" class="btn btn-primary"><i class="fas fa-book"></i> Mnemos</button>
        <button type="button" onclick="window.location='/tech/dls'" class="btn btn-secondary"><i class="fas fa-list"></i> Liste DLS</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

 <div class="row m-2">
   <h5>Du synoptique <strong id="idSourceSynoptique"></strong></h5>
 </div>

<hr>
     <div class="row m-1">
       <div id="idErrorLog" class="col-12 alert alert-info" role="alert">
         <!--<h6 class="text-center">Résultat de compilation</h6>-->
       </div>
     </div>

     <div class="row m-1">
       <div class="col-12 border border-info">
         <textarea id="idSourceCode" rows="10"></textarea>
       </div>
     </div>

<!-- Container -->
</div>

<script src="https://cdnjs.cloudflare.com/ajax/libs/codemirror/5.56.0/codemirror.min.js" type="text/javascript"></script>
<script src="/js/tech/dls_source.js" type="text/javascript"></script>
