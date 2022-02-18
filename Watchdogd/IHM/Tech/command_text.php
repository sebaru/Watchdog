<div class="container-fluid">

 <div class="row m-2">
   <h3><img src="/img/commande_texte.png" style="width:80px" alt="Commandes textuelles">Mapping des Commandes Textuelles</h3>
 </div>

<hr>

<!----------------------------------------------------------------------------------------------------------------------------->
 <div class="row m-2 text-center">
           <label class="col-3 col-form-label text-right">Ajouter une commande textuelle</label>
           <input id="idModalCommandTextAdd" type="text" class="col-4 form-control" placeholder="Nouvelle commande textuelle">
           <button type="button" onclick="COMMAND_TEXT_Add()" class="ml-2 mr-auto btn btn-primary"><i class="fas fa-plus"></i> Ajouter</button>
 </div>

    <table id="idTableTXT" class="table table-striped table-bordered table-hover">
      <thead class="thead-dark">
      </thead>
      <tbody>
      </tbody>
    </table>

<!-- Container -->
</div>

<script src="/js/tech/command_text.js" type="text/javascript"></script>
