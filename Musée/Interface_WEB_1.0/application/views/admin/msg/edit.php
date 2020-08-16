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
						<h3 class="box-title">Edition du message <strong><?php echo $message->tech_id.':'.$message->acronyme ?></strong></h3>
					</div>
					<div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-edit_mnemo')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
            <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
            <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
            <?php echo anchor('admin/dls/sourceedit/'.$message->dls_id, 'Voir la source DLS', array('class' => 'btn btn-primary')); ?>
            <?php echo anchor('admin/msg/index/'.$message->dls_id, 'Retour Liste', array('class' => 'btn btn-default')); ?>
          </div>
        </div>

        <ul class="nav nav-tabs">
          <li class="active"><a data-toggle="tab" href="#general">Général</a></li>
          <li><a data-toggle="tab" href="#audio">Audio</a></li>
          <li><a data-toggle="tab" href="#sms">SMS</a></li>
        </ul>

        <div class="tab-content">
          <div id="general" class="tab-pane fade in active" style="padding:10px">

            <div class="form-group">
              <label class='col-sm-2 control-label'>Synoptique parent</label>
              <div class="col-sm-10"><?php echo form_input($ppage);?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>Synoptique</label>
              <div class="col-sm-10"><?php echo form_input($page);?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>D.L.S Tech_id</label>
              <div class="col-sm-10"><?php echo form_input($dls_tech_id);?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>Acronyme</label>
              <div class="col-sm-10"><?php echo form_input($acronyme);?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>D.L.S</label>
              <div class="col-sm-10"><?php echo form_input($dls_shortname);?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>Type</label>
              <div class="col-sm-10"><?php echo $this->msgs_types[$type];?></div>
            </div>

            <div class="form-group">
              <label class='col-sm-2 control-label'>Libelle</label>
              <div class="col-sm-8"><?php echo form_input($libelle);?></div>
              <div class="col-sm-2"><?php echo form_checkbox($enable);?>Enable</div>
            </div>

          </div>

          <div id="audio" class="tab-pane fade" style="padding:10px">

            <div class="form-group">
              <label class='col-sm-2 control-label'>Libelle Audio</label>
              <div class="col-sm-10"><?php echo form_input($libelle_audio);?></div>
            </div>

          </div>

          <div id="sms" class="tab-pane fade" style="padding:10px">

            <div class="form-group">
              <label class='col-sm-2 control-label'>Mode SMS</label>
              <div class="col-sm-2"><?php echo form_dropdown($sms_mode);?></div>
            </div>
            <div class="form-group">
              <label class='col-sm-2 control-label'>Libelle SMS</label>
              <div class="col-sm-10"><?php echo form_input($libelle_sms);?></div>
            </div>

          </div>

        </div>

        <?php echo form_close();?>

     </div>

    </div>
   </div>
  </div>
 </section>
</div>
