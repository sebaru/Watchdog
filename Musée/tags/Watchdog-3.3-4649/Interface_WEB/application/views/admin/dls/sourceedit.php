<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
	</section>

	<section class="content">

		<div class="row">
			<div class="col-md-12">
				<div class="box box-info">
					<div class="box-header with-border">
						<h3 class="box-title">Edition de la source D.L.S : <strong><?php echo $dls->tech_id.'</strong> - '.$dls->shortname.' - '.$dls->name?></h3>
					</div>
					<div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-sourceedit')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
            <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => '<i class="fa fa-save"></i> Sauvegarder & Compiler')); ?>
            <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => '<i class="fa fa-undo"></i> Annuler')); ?>
            <?php echo anchor('admin/mnemo/index/'.$dls->tech_id, '<i class="fa fa-book"></i> Voir les mnemos', array('class' => 'btn btn-primary')); ?>
            <?php echo anchor('admin/dls/run/'.$dls->tech_id, '<i class="fa fa-eye"></i> Voir le RUN', array('class' => 'btn btn-primary')); ?>
            <?php echo anchor('admin/dls', '<i class="fa fa-list"></i> Retour Liste', array('class' => 'btn btn-default')); ?>
            <?php if ($dls->package!="custom")
                   { echo anchor('admin/dls/download_package/'.$dls->tech_id,
                                 '<i class="fa fa-download"></i> Update Package',
                                 array('class' => 'btn btn-secondary'));
                   } ?>
          </div>
        </div>

        <div class="form-group">
          <div class="col-md-12">
            <?php echo form_textarea($sourcecode) ?>
          </div>
        </div>

        <?php echo form_close();?>
					</div>

     <div class="box-footer">
        <div class="col-md-2">
          <label>Résultat de compilation</label>
         </div>
        <div>
            <textarea readonly rows="10" class="col-md-10"><?php echo $error_log;?></textarea>
        </div>
     </div>
				</div>
			</div>
		</div>
	</section>
</div>
