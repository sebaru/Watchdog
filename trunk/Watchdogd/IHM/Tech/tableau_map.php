<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-chart-line text-primary"></i> Courbes du Tableau <strong id="idTableauTitle"></strong></h3>

   <div class ="ml-auto btn-group align-items-start">
        <button type="button" onclick="Tableau_Map_New()" class="btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
        <button type="button" onclick="Redirect('/tech/tableau')" class="btn btn-secondary"><i class="fas fa-list"></i> Liste</button>
   </div>
 </div>

<hr>

<div class="table-responsive">
  <table id="idTableTableauMap" class="table table-striped table-bordered table-hover">
    <thead class="thead-dark">
				</thead>
			 <tbody>
    </tbody>
  </table>
</div>


<!-- Container -->
</div>

<script src="/js/tech/tableau_map.js" type="text/javascript"></script>
