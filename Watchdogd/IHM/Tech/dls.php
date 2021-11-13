<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-code text-primary"></i> Liste des Modules D.L.S</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Dls_Add()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un DLS</button>
        <button type="button" onclick="Go_to_dls_status()" class="btn btn-info"><i class="fas fa-eye"></i> Statut Global</button>
         <!-- <button type="button" class="btn btn-sm btn-primary rounded-circle"><i class="fas fa-plus"></i></button>-->
   </div>
 </div>

<hr>

    <table id="idTableDLS" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
				  </thead>
			   <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<div id="idModalDlsEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalDlsEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Synoptique du module</label>
						     <select id="idModalDlsEditPage" class="custom-select border-info"></select>
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">TechID du module</label>
						      <input id="idModalDlsEditTechID" type="text" class="form-control" maxlength="32" placeholder="TechID du module DLS">
           </div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">ShortName</label>
						     <input id="idModalDlsEditShortname" type="text" class="form-control" placeholder="Nom court du module DLS">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-5 col-sm-4 col-form-label text-right">Description du module</label>
						     <input id="idModalDlsEditDescription" type="text" class="form-control" placeholder="Description du module D.L.S">
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalDlsEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>


<script src="/js/tech/dls.js" type="text/javascript"></script>
