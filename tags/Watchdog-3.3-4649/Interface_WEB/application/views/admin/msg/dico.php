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
         <h3 class="box-title"><?php echo anchor('admin/msg/create', '<i class="fa fa-gears"></i>Ajouter un message', array('class' => 'btn btn-block btn-primary')); ?></h3>
       </div>

     <div class="box-body">													

       <?php $flash_error=$this->session->flashdata('flash_error');
             if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
       <?php $flash_msg=$this->session->flashdata('flash_message');
             if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

							<table id="msg" class="display responsive">
  								<thead class="thead-dark">
				    					<tr>
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
					     </tbody>
		     </table>
						              
					</div>
				</div>
			</div>
		</div>
	</section>
</div>
