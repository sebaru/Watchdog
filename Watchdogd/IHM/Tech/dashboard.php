    <div class="container-fluid">

 <div class="row m-2">
   <h3><i class="fas fa-tachometer-alt text-primary"></i> Tableau de bord</h3>
 </div>

<hr>

<div class="row justify-content-center row-cols-1 row-cols-sm-2 row-cols-md-3 row-cols-lg-3 row-cols-xl-4">
<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-image text-danger"></i>
        </div>
        <div class="col-8 mt-1"><h5>Synoptiques</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text"><span id="idNbrSyns">-</span> Synoptiques</h6></li>
      <li><h6 class="card-text"><span id="idNbrSynsVisuels">-</span> Motifs</h6></li>
      <li><h6 class="card-text"><span id="idNbrSynsLiens">-</span> Liens</h6></li>
      </ul>
      <!--<h3 class="card-text text-center"><strong>56</h3>-->
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>
<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-code text-primary"></i>
        </div>
        <div class="col-8 mt-1"><h5>D.L.S</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text"><span id="idNbrDls">-</span> Modules</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsLignes">-</span> Lignes</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsBI">-</span> Bistables</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsMONO">-</span> Monostables</h6></li>
      <li><h6 class="card-text"><span id="idDlsBitparsec">-</span> bits par seconde</h6></li>
      <li><h6 class="card-text"><span id="idDlsTourparsec">-</span> tours par seconde</h6></li>
      <!--<h3 class="card-text text-center"><strong>56</h3>-->
      </ul>
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>

<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-crown text-info"></i>
        </div>
        <div class="col-8 mt-1"><h5>Instance</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text">Version: <span id="idConfigVersion">-</span></h6></li>
      <li><h6 class="card-text">Instance: <span id="idConfigInstance">-</span></h6></li>
      <li><h6 class="card-text">Master: <span id="idConfigMaster">-</span></h6></li>
      <li><h6 class="card-text">Master Host: <span id="idConfigMasterHost">-</span></h6></li>
      <li><h6 class="card-text">Started: <span id="idConfigStarted">-</span></h6></li>
      <li><h6 class="card-text">Run_as:<span id="idConfigRunAs">-</span></h6></li>
      <li><h6 class="card-text">Top: <span id="idConfigTop">-</span></h6></li>
      <!--<h3 class="card-text text-center"><strong>56</h3>-->
      </ul>
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>

<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-cogs text-warning"></i>
        </div>
        <div class="col-8 mt-1"><h5>Entrées/Sorties</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text"><span id="idNbrDlsDI">-</span> Entrées TOR</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsAI">-</span> Entrées ANA</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsDO">-</span> Sorties TOR</h6></li>
      <li><h6 class="card-text"><span id="idNbrDlsAO">-</span> Sorties ANA</h6></li>
      <!--<h3 class="card-text text-center"><strong>56</h3>-->
      </ul>
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>
<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-envelope text-success"></i>
        </div>
        <div class="col-8 mt-1"><h5>Messages</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text"><span id="idNbrMsgs">-</span> Messages</h6></li>
      <li><h6 class="card-text"><span id="idNbrHistoMsgs">-</span> messages dans l'historique</h6></li>
      </ul>
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>
<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-users text-primary"></i>
        </div>
        <div class="col-8 mt-1"><h5>Utilisateurs</h5></div>
      </div>
    </div>
    <div class="card-body">
      <ul>
      <li><h6 class="card-text"><span id="idNbrUsers">-</span> Utilisateurs</h6></li>
      <li><h6 class="card-text"><span id="idNbrAuditLog">-</span> Enregistrements Logs</h6></li>
      <li><h6 class="card-text"><span id="idNbrSessions">-</span> Sessions</h6></li>
      </ul>
<!--<h3 class="card-text text-center"><strong>56</h3>-->
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>


<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-database text-primary"></i>
        </div>
        <div class="col-8 mt-1"><h5>Database</h5></div>
      </div>
    </div>
    <div class="card-body">

        <ul>
          <li><h6 class="card-text">DB Username: <span id="idDBUsername">-</span></h6></li>
          <li><h6 class="card-text">DB Hostname: <span id="idDBHostname">-</span></h6></li>
          <li><h6 class="card-text">DB Port: <span id="idDBPort">-</span></h6></li>
          <li><h6 class="card-text">DB Database: <span id="idDBDatabase">-</span></h6></li>
        </ul>
<!--<h3 class="card-text text-center"><strong>56</h3>-->
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>


<div class="col p-1">
  <div class="card h-100">
    <div class="card-header bg-highlight">
      <div class="row">
        <div class="col-4">
          <i class="fas fa-2x fa-database text-secondary"></i>
        </div>
        <div class="col-8 mt-1"><h5>Arch Database</h5></div>
      </div>
    </div>
    <div class="card-body">
        <ul>
          <li><h6 class="card-text">DB Username: <span id="idArchDBUsername">-</span></h6></li>
          <li><h6 class="card-text">DB Hostname: <span id="idArchDBHostname">-</span></h6></li>
          <li><h6 class="card-text">DB Port: <span id="idArchDBPort">-</span></h6></li>
          <li><h6 class="card-text">DB Database: <span id="idArchDBDatabase">-</span></h6></li>
          <li><h6 class="card-text">Nbr Enreg: <span id="idArchDBNbrEnreg">-</span></h6></li>
        </ul>
<!--<h3 class="card-text text-center"><strong>56</h3>-->
    </div>
    <!--<p class="card-text text-center"><small class="text-muted">Last updated 3 mins ago</small></p>-->
  </div>
</div>
</div>

<script src="/js/tech/dashboard.js" type="text/javascript"></script>
<!-- Container -->
</div>
