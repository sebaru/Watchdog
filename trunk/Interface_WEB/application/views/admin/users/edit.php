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
						<h3 class="box-title">Edition de l'utilisateur <strong><?php echo $user->username;?></strong></h3>
					</div>
					<div class="box-body">

        <?php $flash_error=$this->session->flashdata('flash_error');
        if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
        <?php $flash_msg=$this->session->flashdata('flash_message');
        if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

        <?php echo form_open(uri_string(), array('class' => 'form-horizontal', 'id' => 'form-edit_user')); ?>

        <div class="col-md-12 form-group">
          <div class="btn-group pull-right">
              <?php echo form_button(array('type' => 'submit', 'class' => 'btn btn-success', 'content' => lang('actions_submit'))); ?>
              <?php echo form_button(array('type' => 'reset', 'class' => 'btn btn-warning', 'content' => lang('actions_reset'))); ?>
              <?php echo anchor('admin/users', 'Retour Liste', array('class' => 'btn btn-default')); ?>
          </div>
        </div>

        <div class="form-group">
          <label for='user_name' class='col-sm-2 control-label'>Username</label>
          <div class="col-sm-5"><?php echo form_input($user_name);?></div>
          <label for="access_level" class="col-sm-2 control-label">Niveau de privilèges</label>
          <div class="col-sm-3">  
            <?php if ($disabled=="TRUE") { ?>
              <select disabled name="access_level" class="form-control">
                <option><?php echo $access_level['value']?></option>
              </select>
            <?php } else { ?>
	             <select name="access_level" class="form-control">
	               <?php for ($i=0;$i<$access;$i++)
                       { if ($access_level['value']==$i) $selected="selected";else $selected='';?>
	                       <option value="<?php echo $i?>" <?php echo $selected?>><?php echo $i?></option>
                <?php  }?>
              </select>
            <?php } ?>
          </div>
        </div>

        <div class="form-group">
          <label for='comment' class='col-sm-2 control-label'>Description</label>
          <div class="col-sm-10"><?php echo form_input($comment);?></div>
        </div>

        <div class="form-group">
          <label for='sms_phone' class='col-sm-2 control-label'>N° de téléphone</label>
          <div class="col-sm-10"><?php echo form_input($sms_phone);?></div>
        </div>

        <div class="form-group">
          <label for='email' class='col-sm-2 control-label'>Adresse de messagerie</label>
          <div class="col-sm-10"><?php echo form_input($email);?></div>
        </div>

        <div class="form-group">
          <label class='col-sm-2 control-label'>Messagerie Instantanée</label>
          <div class="col-sm-6"><?php echo form_input($imsg_jabberid);?></div>
          <div class="col-sm-2 "><?php echo form_checkbox($imsg_enable);?>Notification</div>
          <div class="col-sm-2 "><?php echo form_checkbox($imsg_allow_cde);?>Commande à distance</div>
        </div>

        <div class="form-group">
          <?php echo lang('users_password', 'password', array('class' => 'col-sm-2 control-label')); ?>
          <div class="col-sm-10">
             <?php echo form_input($password);?>
               <div class="progress" style="margin:0">
                 <div class="pwstrength_viewport_progress"></div>
               </div>
          </div>
        </div>

        <div class="form-group">
          <?php echo lang('users_password_confirm', 'password_confirm', array('class' => 'col-sm-2 control-label')); ?>
            <div class="col-sm-10"><?php echo form_input($password_confirm);?></div>
        </div>

        <?php echo form_close();?>

     </div>

    </div>
   </div>
  </div>
 </section>
</div>
