<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">
	<section class="content-header">
		<?php echo $pagetitle; ?>
	</section>

	<section class="content">
		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="col-md-12 form-group">
           <div class="btn-group pull-right">
               <?php echo anchor('admin/dls/run', '<i class="fa fa-eye"></i> Voir le RUN', array('class' => 'btn btn-primary')); ?>
           </div>
         </div>
       </div>

     <div class="box-body">

       <?php $flash_error=$this->session->flashdata('flash_error');
             if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
       <?php $flash_msg=$this->session->flashdata('flash_message');
             if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>


     							<table id="dls" class="display responsive nowrap" style="width:100%;">
     								<thead class="thead-dark">
				    					<tr>
	    									<th scope="col">Actions</th>
								    		<th scope="col">Enable</th>
								    		<th scope="col">Tech ID</th>
								    		<th scope="col">Package</th>
					  	 	 			<th scope="col">Groupe/<br>Page</th>
				  	  					<th scope="col">Short name</th>
				    						<th scope="col">Name</th>
			    							<th scope="col">Compil Date</th>
		    								<th scope="col">Compil Status</th>
	    									<th scope="col"># Lines/Compil</th>
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
