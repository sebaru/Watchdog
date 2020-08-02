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

       <!-- <div class="box-header with-border">
         <h3 class="box-title"><?php echo anchor('admin/msg/create', '<i class="fa fa-gears"></i>Ajouter un message', array('class' => 'btn btn-block btn-primary')); ?></h3>
       </div>-->

     <div class="box-body">													

       <?php $flash_error=$this->session->flashdata('flash_error');
             if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
       <?php $flash_msg=$this->session->flashdata('flash_message');
             if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

							<table class="table table-striped table-hover">
  								<thead class="thead-dark">
				    					<tr>
                <th>Actions</th>
                <th>Enable</th>
                <th>DLS shortname</th>
                <th>Tech ID</th>
                <th>Acronyme</th>
                <th>Type</th>
                <th>Message</th>                                                
                <th>Notif SMS</th>
                <th>Notif Audio</th>
                <th>Repeat ?</th>
		    							</tr>
			     		</thead>
					     <tbody>
            <?php foreach ($msgs as $msg):?>
              <tr>
                <td><div class="dropdown">
                      <button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">
                       <span class="caret"></span>
                      </button>
                      <ul class="dropdown-menu">
                        <li><a href=<?php echo site_url('admin/msg/edit/').$msg->id; ?>>
                          <i class="fa fa-pencil" style="color:green"></i>
                          <i class="fa fa-envelope" style="color:blue"></i>Editer</a></li>
                        <li class="divider"></li>
                        <li><a href=<?php echo site_url('admin/msg/delete/').$msg->id; ?>>
                           <i class="fa fa-times" style="color:red"></i>Supprimer</a></li>
                      </ul>
                    </div>
                </td>
                <td><?php echo $msg->enable ? anchor('admin/msg/deactivate/'.$msg->id, '<span class="label label-success">Activé</span>')
                                            : anchor('admin/msg/activate/'.$msg->id, '<span class="label label-default">Inactif</span>'); ?></td>
                <td><?php echo $msg->shortname;?></td>
                <td><?php echo $msg->tech_id;?></td>
                <td><?php echo anchor('admin/msg/edit/'.$msg->id, $msg->acronyme);?></td>
                <td><?php echo $this->msgs_types[$msg->type];?></td>
                <td><?php echo anchor('admin/msg/edit/'.$msg->id, $msg->libelle);?></td>
                <td><?php echo $this->msgs_types_sms[$msg->sms];?></td>
                <td><?php echo $msg->audio ? '<span class="label label-success">Activé</span>'
                                           : '<span class="label label-default">Inactif</span>';?></td>
                <td><?php echo $msg->time_repeat;?></td>
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
