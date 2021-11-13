<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-mimage text-primary"></i> Liste des Synoptiques</strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Show_Modal_Syn_Add(0)" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter un Synoptique</button>
   </div>
 </div>

<hr>

<div class="table-responsive">
  <table id="idTableSyn" class="table table-striped table-bordered table-hover">
    <thead class="thead-dark">
				</thead>
			 <tbody>
    </tbody>
  </table>
</div>


<!-- Container -->
</div>


<div id="idModalSynEdit" class="modal fade" tabindex="-1" role="dialog">
  <div class="modal-dialog modal-dialog-centered modal-lg" role="document">
    <div class="modal-content ">
      <div class="modal-header bg-info text-white">
        <h5 class="modal-title text-justify"><i class="fas fa-pen"></i> <span id="idModalSynEditTitre"></span></h5>
        <button type="button" class="close" data-dismiss="modal" aria-label="Close">
          <span aria-hidden="true">&times;</span>
        </button>
      </div>
      <div class="modal-body">

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Page du Parent</label>
						     <select id="idModalSynEditPPage" class="col-9 custom-select border-info"></select>
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Nom de la Page</label>
						     <input id="idModalSynEditPage" type="text" class="form-control" placeholder="Titre du synoptique">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right"><i class="fas fa-star"></i> Level</label>
						     <input id="idModalSynEditAccessLevel" type="number" class="form-control" min=0 max=9 placeholder="Level">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Description du synoptique</label>
						     <input id="idModalSynEditDescription" type="text" class="form-control" placeholder="Description du synoptique">
     					</div>
       </div>

       <div class="col form-group">
					     <div class="input-group align-items-center">
						     <label class="col-4 col-form-label text-right">Image pré-établie</label>
						     <select id="idModalSynEditImageSelect" class="col-6 form-control-file custom-select">
             <option value='custom'>custom</option>
             <option value='home'>Accueil</option>
             <option value='comm'>Communications</option>
             <option value='confort'>Confort</option>
             <option value='cuisine'>Cuisine</option>
           </select>
           <img id="idModalSynEditImage" src='' class="ml-2 text-center col-2 border border-info">
     					</div>
       </div>

       <div class="col form-group align-items-center">
					     <div class="input-group">
						     <label class="col-4 col-form-label text-right">Image du synoptique</label>
						     <input id="idModalSynEditImageCustom" type="file" accept="image/jpg" class="col-8 form-control-file" placeholder="Choisissez une image">
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

<script src="/js/tech/synoptiques.js" type="text/javascript"></script>
