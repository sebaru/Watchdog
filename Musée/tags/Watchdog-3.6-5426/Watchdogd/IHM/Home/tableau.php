<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-chart-line text-primary"></i> Tableau: <strong id="idTableauTitle"></strong></h3>

   <div class ="ml-auto btn-group align-items-start">
      <i class="fas fa-clock text-primary mt-2 mr-2"></i>
        <select id='idTableauPeriod' class='custom-select' onchange="Tableau_Set_Period()">
          <option value='HOUR'>Heure</option>
          <option value='DAY'>Jour</option>
          <option value='WEEK'>Semaine</option>
          <option value='MONTH'>Mois</option>
          <option value='YEAR'>Année</option>
          <option value='ALL'>Tout</option>
        </select>
   </div>
 </div>

<hr>

  <div class="row m-1 border border-info">
     <canvas id="idChart" class="col"></canvas>
  </div>

<!-- Container -->
</div>

<script src="/js/home/tableau.js" type="text/javascript"></script>
