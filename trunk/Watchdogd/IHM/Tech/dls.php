<div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-code text-primary"></i> Liste des Modules D.L.S</h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Dls_Compiler_tous_dls()" class="btn btn-outline-success"><i class="fas fa-coffee"></i> Tout Compiler</button>
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

<div id="idModalSynEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalSynEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Parent</span>
						     </div>
						     <input id="idModalSynEditPPage" type="text" class="form-control" placeholder="Parent du synoptique">
     					</div>
        </div>

        <div class="form-row form-group">

					     <div class="col-7 input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Page</span>
						     </div>
						     <input id="idModalSynEditPage" type="text" class="form-control" placeholder="Titre du synoptique">
     					</div>

					     <div class="col-5 input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text"><i class="fas fa-star"></i></span>
						     </div>
						     <input id="idModalSynEditAccessLevel" type="number" class="form-control" min=0 max=9 placeholder="Level">
     					</div>

        </div>

        <div class="form-group">
					     <div class="input-group">
						     <div class="input-group-prepend">
							     <span class="input-group-text">Description</span>
						     </div>
						     <input id="idModalSynEditDescription" type="text" class="form-control" placeholder="Description du synoptique">
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalSynEditValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>

<div id="idModalDlsAdd" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span>Ajouter un module D.L.S</strong></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Synoptique du module</label>
						     <select id="idModalDlsAddPage" class="custom-select border-info"></select>
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">TechID du module</label>
						     <input id="idModalDlsAddTechID" type="text" class="form-control" placeholder="TechID du module DLS">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Description du module</label>
						     <input id="idModalDlsAddDescription" type="text" class="form-control" placeholder="Description du module D.L.S">
     					</div>
        </div>

      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-secondary" data-dismiss="modal"><i class="fas fa-times"></i> Annuler</button>
        <button id="idModalDlsAddValider" type="button" class="btn btn-primary" data-dismiss="modal"><i class="fas fa-save"></i> Valider</button>
      </div>
    </div>
  </div>
</div>


<script src="/js/tech/dls.js" type="text/javascript"></script>
