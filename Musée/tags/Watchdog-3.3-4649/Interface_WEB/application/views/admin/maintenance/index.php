<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
         Maintenance
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         Actions de maintenance
         <div class="label-group pull-right">
         </div>
       </div>


				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>


    		<div class="col-md-12">
         <div class="col-md-2"><label>Archiver les messages alive</label></div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/clear_histo', '<span class="label label-warning">Archiver</span>');?>
         </div>
         <div class="col-md-3">
         </div>
      </div>

    		<div class="col-md-12">
         <div class="col-md-2"><label>Changer le niveau de log</label></div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/log/debug', '<span class="label label-info">Debug</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/log/info', '<span class="label label-success">Info</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/log/notice', '<span class="label label-primary">Notice</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/log/warning', '<span class="label label-warning">Warning</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/log/error', '<span class="label label-danger">Error</span>');?>
         </div>
      </div>

    		<div class="col-md-12">
         <div class="col-md-2"><label>Gestion des archives</label></div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/archive/clear', '<span class="label label-warning">Clear</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/archive/purge', '<span class="label label-info">Purge Database</span>');?>
         </div>
         <div class="col-md-1">
           <?php echo anchor('admin/maintenance/archive/testdb', '<span class="label label-success">Test DB Connect</span>');?>
         </div>
      </div>

   <!--      <form method="post" action=<?php echo site_url("/admin/process/clear_histo") ?>>
           <?php echo form_submit( "submit", "Changer", array('class' => 'label label-primary') )?>

         </form>-->

				</div>
				</div>
			</div>
		</div>
	</section>
</div>
