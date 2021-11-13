					<!--<img src="<?=base_url()?>/assets/img/logo_b.png" width=160>-->


<div class="container">
	<div class="d-flex justify-content-center h-100">
		<div class="card">
			<div class="card-header">
     <div class="row">
       <img class="col-4" src="<?php echo base_url('/logo.svg')?>" alt="Watchdog Logo">
       <h4 class="col-8 align-self-center text-center wtd_title">Connexion</h4>
     </div>
			</div>
			<div class="card-body">

					<div class="input-group form-group">
						<div class="input-group-prepend">
							<span class="input-group-text"><i class="fas fa-user"></i></span>
						</div>
						<input id="username" name="username" type="text" class="form-control" placeholder="username">
					</div>

					<div class="input-group form-group">
						<div class="input-group-prepend">
							<span class="input-group-text"><i class="fas fa-key"></i></span>
						</div>
						<input id="password" name="password" type="password" class="form-control" placeholder="password">
					</div>

					<div class="form-group">
						<button onclick="Send_credential()" class="btn float-right btn-primary">Entrer</button>
					</div>
			</div>
			<div class="card-footer">
				<div class="d-flex justify-content-center">
					<a class="card-link" href="https://wiki.abls-habitat.fr"><i class="fas fa-info-circle"></i> Go to ABLS Wiki !</a>
				</div>
			</div>
		</div>
	</div>


<!-- <div id="toast-error" class="toast" role="alert" aria-live="assertive" aria-atomic="true" style="position: absolute; top: 0; center: 0;">
   <div class="toast-header">
     <i class="fas exclamation-circle"></i>
     <strong class="mr-auto">Bootstrap</strong>
     <small>11 mins ago</small>
     <button type="button" class="ml-2 mb-1 close" data-dismiss="toast" aria-label="Close">
       <span aria-hidden="true">&times;</span>
     </button>
   </div>
   <div class="toast-body">
     Hello, world! This is a toast message.
   </div>
 </div>
-->

</div>
