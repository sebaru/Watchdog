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

					<div class="box-body">

       <?php $flash_error=$this->session->flashdata('flash_error');
             if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
       <?php $flash_msg=$this->session->flashdata('flash_message');
             if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

							<table id="syns" class="display responsive nowrap" style="width:100%;">
								<thead class="thead-dark">
									<tr>
										<th scope="col" class="hidden-xs">Actions</th>
										<th scope="col" class="hidden-xs">#</th>
										<th scope="col" class="hidden-xs">Access Level</th>
										<th scope="col" class="hidden-xs">Synoptique Parent</th>
										<th scope="col">Page</th>
										<th scope="col">Libelle</th>
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
