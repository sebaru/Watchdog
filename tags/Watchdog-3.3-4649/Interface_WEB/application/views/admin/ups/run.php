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
         <div class="box-title btn-group pull-right">
             <?php echo anchor('admin/ups/index', '<i class="fa fa-battery"></i>Retour Liste', array('class' => 'btn btn-primary')); ?>
         </div>
       </div>

				<div class="box-body">

      <?php $flash_error=$this->session->flashdata('flash_error');
            if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
      <?php $flash_msg=$this->session->flashdata('flash_message');
            if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

      <table class="table table-striped table-hover">
        <thead>
          <tr>
            <th>Tech_ID</th>
            <th>Started</th>
            <th>Comm</th>
            <th>Load</th>
            <th>Realpower</th>
            <th>Batt.Charge</th>
            <th>Batt.RunTime</th>
            <th>Batt.Voltage</th>
            <th>Input.Voltage</th>
            <th>Input.HZ</th>
            <th>Output.Current</th>
            <th>Output.Voltage</th>
            <th>Output.HZ</th>
            <th>Outlet1</th>
            <th>Outlet2</th>
            <th>Online</th>
            <th>Charging</th>
            <th>On Batt</th>
          </tr>
        </thead>
        <tbody>
          <?php foreach ($onduleurs as $tech_id => $onduleur):?>
            <tr>
              <td><?php echo $tech_id; ?></td>
              <td><?php echo $onduleur->started; ?></td>
              <td><?php echo $onduleur->comm; ?></td>
              <td><?php echo $onduleur->ups_load; ?></td>
              <td><?php echo $onduleur->ups_realpower; ?></td>
              <td><?php echo $onduleur->ups_battery_charge; ?></td>
              <td><?php echo $onduleur->ups_battery_runtime; ?></td>
              <td><?php echo $onduleur->ups_battery_voltage; ?></td>
              <td><?php echo $onduleur->ups_input_voltage; ?></td>
              <td><?php echo $onduleur->ups_input_frequency; ?></td>
              <td><?php echo $onduleur->ups_output_current; ?></td>
              <td><?php echo $onduleur->ups_output_voltage; ?></td>
              <td><?php echo $onduleur->ups_output_frequency; ?></td>
              <td><?php echo $onduleur->ups_outlet_1_status; ?></td>
              <td><?php echo $onduleur->ups_outlet_2_status; ?></td>
              <td><?php echo $onduleur->ups_online; ?></td>
              <td><?php echo $onduleur->ups_charging; ?></td>
              <td><?php echo $onduleur->ups_on_batt; ?></td>
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
