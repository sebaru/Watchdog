<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">

	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
		       <h3 class="box-title">Messages au fil de l'eau</h3>
         <div class="btn-group pull-right">

          <a href="<?php echo site_url('msgs'); ?>" class="btn btn-primary">
            <img src="https://icons.abls-habitat.fr/assets/gif/Pignon_vert.svg" style="width:20px" />
                 Tous les messages
          </a>

          <a href="<?php echo site_url('msgs/activite'); ?>" class="btn btn-primary">
            <img src="https://icons.abls-habitat.fr/assets/gif/Pignon_orange.svg" style="width:20px" />
                 Défauts et Alarmes
          </a>

          <a href="<?php echo site_url('msgs/secubien'); ?>" class="btn btn-primary">
            <img src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_rouge.svg" style="width:20px" />
                 Veilles et Alertes
          </a>

          <a href="<?php echo site_url('msgs/secupers'); ?>" class="btn btn-primary">
            <img src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_rouge.svg" style="width:20px" />
                 Dangers et Dérangements
          </a>

         </div>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table id="msgfdl" class="table table-striped table-hover">
        <thead>
          <tr>
            <th class="hidden-xs">Date</th>
            <th>Type</th>
            <th>D.L.S</th>
            <th>Message</th>
            <th class="hidden-xs">Acquit</th>
            <th class="hidden-xs">Date Acquit</th>
            <th class="hidden-xs">Acquitter</th>
          </tr>
        </thead>
        <tbody>
        </tbody>
      </table>

				</div>
				</div>
			</div>
		</div>
	</section>
</div>
