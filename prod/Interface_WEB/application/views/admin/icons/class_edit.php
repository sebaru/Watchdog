<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
		<?php echo $breadcrumb; ?>	
	</section>

	<section class="content">

		<div class="row">
			<div class="col-md-12">
				<div class="box box-info">
					<div class="box-header with-border">
						<h3 class="box-title">Editer la classe <strong><?php echo $class->libelle; ?></strong></h3>
					</div>
					<div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-class_edit')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
              <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
              <?php echo anchor('admin/icons', 'Retour Liste', array('class' => 'btn btn-default')); ?>
          </div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Libellé de la classe</label>
          <div class="col-sm-10"><?php echo form_input($libelle);?></div>
        </div>

        <?php echo form_close();?>

     </div>

    </div>
   </div>
  </div>
 </section>
</div>
