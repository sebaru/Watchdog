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
		       <h3 class="box-title">Recherche de messages dans l'historique</h3>
       			<div class="col-md-12">
           <form method="POST">
            <div class="col-md-7"></div>
            <div class="col-md-2 control-label"><label>Elements de recherche</label></div>
            <div class="col-md-2"><input type="text" id="search" name="search"></div>
            <div class="col-md-1"><input type="submit" value="Rechercher"></div>
           </form>
          </div>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table id="msghisto" class="table table-striped table-hover">
        <thead>
          <tr>
            <th class="hidden-xs">Date</th>
            <th>Type</th>
            <th>D.L.S</th>
            <th>Message</th>
            <th class="hidden-xs">Acquit</th>
            <th class="hidden-xs">Date Acquit</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($msgs as $msg):?>
            <tr>
              <td class="hidden-xs"><?php echo $msg->date_create; ?></td>
              <td><?php echo $this->Msg_model->type_to_icone($msg->type); ?></td>
              <td><?php echo $msg->shortname; ?></td>
              <td><?php echo $msg->libelle; ?></td>
              <td class="hidden-xs"><?php echo $msg->nom_ack; ?></td>
              <td class="hidden-xs"><?php if ($msg->nom_ack!="None") echo $msg->date_fixe; ?></td>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>

				</div>
				</div>
			</div>
		</div>
	</section>
</div>
