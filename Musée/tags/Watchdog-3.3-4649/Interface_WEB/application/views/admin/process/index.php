<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
  <?php echo $breadcrumb; ?>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="btn-group pull-right">
         </div>
         <form method="post" action=<?php echo site_url("/admin/process/set_instance") ?>>
         <div class="col-md-2"><label>Liste des instances</label></div>
         <div class="col-md-3">
           <select name="instance" class="form-control">
             <?php foreach ($instances as $index => $instance):
                   if ($this->wtd_webservice->instance() == $instance->instance_id) $selected="selected"; else $selected='';
                   echo "<option value=".$index." ".$selected.">".$instance->instance_id."</option>";
                   endforeach;
             ?>
           </select>
         </div>
         <div class="col-md-1">
           <?php echo form_submit( "submit", "Changer", array('class' => 'btn btn-primary') )?>
         </div>
         </form>
       </div>


				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>Thread</th>
            <th>Started</th>
            <th>Debug</th>
            <th class="hidden-xs">Object</th>
            <th class="hidden-xs">File</th>
            <th>Reload</th>
            <th class="hidden-xs">Documentation</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($processus as $tech_id => $process):?>
            <tr>
              <td><?php echo anchor('admin/config/index/'.$this->wtd_webservice->instance().'/'. $tech_id, $tech_id); ?></td>
              <td><?php if ($process->started)
                       { echo anchor('admin/process/stop/'. $tech_id, '<span class="label label-success">Actif</span>'); }
                  else { echo anchor('admin/process/start/'.$tech_id, '<span class="label label-default">Inactif</span>'); }
                  ?></td>
              <td><?php if ($process->debug)
                       { echo anchor('admin/process/undebug/'. $tech_id, '<span class="label label-success">Actif</span>'); }
                  else { echo anchor('admin/process/debug/'.$tech_id, '<span class="label label-default">Inactif</span>'); }
                  ?></td>
              <td class="hidden-xs"><?php echo anchor('admin/config/index/'.$this->wtd_webservice->instance().'/'. $tech_id, $process->objet); ?></td>
              <td class="hidden-xs"><?php echo $process->fichier; ?></td>
              <td><?php echo anchor('admin/process/reload/'. $tech_id, '<span class="label label-warning">Reload</span>');?></td>
              <td class="hidden-xs"><?php echo anchor('https://wiki.abls-habitat.fr/index.php?title=WatchdogServer_'.strtoupper($tech_id).'_Thread',
                                                      '<span class="label label-info">Documentation</span>'); ?></td>
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
