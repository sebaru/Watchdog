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
         <div class="btn-group pull-right">
             <?php echo anchor('admin/dls', 'Retour Liste', array('class' => 'btn btn-default')); ?>
         </div>
       </div>

				<div class="box-body">
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree.png" />Entrées TOR</a></h2>

      <?php if (array_key_exists('0', $plugin->DI)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->DI[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->DI as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree_Analogique.png" />Entrées ANA</a></h2>

      <?php if (array_key_exists('0', $plugin->AI)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->AI[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->AI as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" />Sorties TOR</a></h2>
      <?php if (array_key_exists('0', $plugin->DO)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->DO[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->DO as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Monostables et Bistables"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" />Monostables et Bistables</a></h2>
      <?php if (array_key_exists('0', $plugin->BOOL)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->BOOL[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->BOOL as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Registres"
                       src="https://icons.abls-habitat.fr/assets/gif/Calculatrice.png" />Registres</a></h2>
      <?php if (array_key_exists('0', $plugin->R)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->R[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->R as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Compteurs d'impulsions"
                       src="https://icons.abls-habitat.fr/assets/gif/Front_montant.png" />Compteurs d'impulsions</a></h2>
      <?php if (array_key_exists('0', $plugin->CI)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->CI[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->CI as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Compteurs horaires"
                       src="https://icons.abls-habitat.fr/assets/gif/Compteur_horaire.png" />Compteurs horaires</a></h2>
      <?php if (array_key_exists('0', $plugin->CH)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->CH[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->CH as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Temporisations"
                       src="https://icons.abls-habitat.fr/assets/gif/Sablier.png" />Temporisations</a></h2>
      <?php if (array_key_exists('0', $plugin->T)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->T[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->T as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->
      <h2><img style="width: 30px" data-toggle="tooltip" title="Messages"
                       src="https://icons.abls-habitat.fr/assets/gif/Message.png" />Messages </a></h2>
      <?php if (array_key_exists('0', $plugin->MSG)) { ?>
      <table class="table table-striped table-hover">
        <thead>
          <?php foreach ($plugin->MSG[0] as $key => $bit):?>
            <th><?php echo $key; ?></th>
          <?php endforeach;?>
        </thead>
        <tbody>
          <?php foreach ($plugin->MSG as $bit):?>
            <tr>
              <?php foreach ($bit as $key => $valeur):?>
                <td><?php echo $valeur; ?></td>
              <?php endforeach;?>
            </tr>
          <?php endforeach;?>
        </tbody>
      </table>
      <hr>
      <?php } ?>
<!----------------------------------------------------------------------------------------------------------------------------->

				</div>
				</div>
			</div>
		</div>
	</section>
</div>
