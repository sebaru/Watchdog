<?php
defined('BASEPATH') OR exit('No direct script access allowed');
?>

<div class="content-wrapper">

	<section class="content">

		<div class="row">
			<div class="col-md-12">
				<div class="box">

       <div class="box-header with-border">
         <div class="box-title btn-group pull-right">
             <?php echo anchor($url_hour, '<i class="fa fa-clock-o"></i> Heure', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor($url_day, '<i class="fa fa-clock-o"></i> Jour', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor($url_week, '<i class="fa fa-clock-o"></i> Semaine', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor($url_month, '<i class="fa fa-clock-o"></i> Mois', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor($url_year, '<i class="fa fa-clock-o"></i> Année', array('class' => 'btn btn-primary')); ?>
         </div>
       </div>

       <div class="box-body">
    			  <div class="col-md-12">
           <canvas id="myTableau" height="100%"> </canvas>
					    </div>
					  </div>

				</div>
			</div>
		</div>
	</section>

</div>
