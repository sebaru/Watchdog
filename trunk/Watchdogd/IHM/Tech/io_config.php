<div class="container">

 <div class="row m-2">
   <h3><i class="fas fa-cogs text-primary"></i> Configuration des Entrées/Sorties</h3>
 </div>

<hr>

<div class="row justify-content-center row-cols-2 row-cols-sm-2 row-cols-md-3 row-cols-lg-3 row-cols-xl-4">

 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/modbus_map"><img src="/img/wago_750342.webp" class="card-img-top" alt="Wago 750-342"></a>
   <div class="card-header">Les I/O Wago</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/modbus')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" onclick="Redirect('/tech/modbus_map')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>

 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/smsg_map"><img src="/img/sms.jpg" class="card-img-top" alt="Commandes SMS"></a>
   <div class="card-header">Les GSM</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/smsg')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" onclick="Redirect('/tech/smsg_map')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>

 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/"><img src="/img/commande_vocale.jpg" class="card-img-top" alt="Commandes Vocales"></a>
   <div class="card-header">Les I/O Vocales</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/ups')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" disabled class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>

 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/ups"><img src="/img/onduleur.jpg" class="card-img-top" alt="Onduleurs"></a>
   <div class="card-header">Les Onduleurs</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/ups')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" disabled class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>

 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/teleinfoedf"><img src="/img/linky.jpg" class="card-img-top" alt="Compteurs EDF"></a>
   <div class="card-header">Les Compteurs EDF</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/teleinfoedf')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" disabled class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>


 <div class="col p-1">
  <div class="card h-100 text-center bg-light">
   <a href="/tech/ephemeride"><img src="/img/ephemeride.svg" height="273px" class="card-img-top" alt="Compteurs EDF"></a>
   <div class="card-header">L'éphéméride</div>
   <div class="card-body">
        <button type="button" onclick="Redirect('/tech/ephemeride')" class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-list"></i> Configurer</button>
        <button type="button" disabled class="btn btn-block m-1 btn-primary btn-sm"><i class="fas fa-directions"></i> Mapper</button>
   </div>
  </div>
 </div>


</div>

</div>


<!--<script src="/js/tech/smsg.js" type="text/javascript"></script>-->
