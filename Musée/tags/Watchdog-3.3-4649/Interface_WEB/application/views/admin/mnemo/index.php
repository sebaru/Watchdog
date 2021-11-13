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
            <h3 class="box-title">Liste des mnémoniques du module D.L.S <strong><?php echo $dls->tech_id.'</strong> - '.$dls->shortname.' - '.$dls->name?></h3>
          </div>
          <div class="box-body">
           <?php $flash_error=$this->session->flashdata('flash_error');
           if ($flash_error) { ?><div class="callout callout-danger"><p><?php echo $flash_error;?></p></div> <?php } ?>
           <?php $flash_msg=$this->session->flashdata('flash_message');
           if ($flash_msg) { ?><div class="callout callout-success"><p><?php echo $flash_msg;?></p></div> <?php } ?>

          <div class="col-md-12 form-group">
            <div class="btn-group pull-right">
              <?php echo anchor('admin/dls/sourceedit/'.$dls->tech_id, '<i class="fa fa-code"></i> Voir la source DLS', array('class' => 'btn btn-primary')); ?>
             <?php echo anchor('admin/dls/run/'.$dls->tech_id, '<i class="fa fa-eye"></i> Voir le RUN', array('class' => 'btn btn-primary')); ?>
            </div>
          </div>

          <ul class="nav nav-tabs">
            <li><a data-toggle="tab" href="#entreetor">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree.png" />Entrées TOR</a></li>
            <li><a data-toggle="tab" href="#entreeana">
                  <img style="width: 30px" data-toggle="tooltip" title="Entrées ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Entree_Analogique.png" />Entrées ANA</a></li>
            <li><a data-toggle="tab" href="#sortietor">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties TOR"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" />Sorties TOR</a></li>
            <li><a data-toggle="tab" href="#sortieana">
                  <img style="width: 30px" data-toggle="tooltip" title="Sorties ANA"
                       src="https://icons.abls-habitat.fr/assets/gif/Sortie_Analogique.png" />Sorties ANA</a></li>
            <li><a data-toggle="tab" href="#registre">
                  <img style="width: 30px" data-toggle="tooltip" title="Registres"
                       src="https://icons.abls-habitat.fr/assets/gif/Calculatrice.png" />Registres</a></li>
            <li><a data-toggle="tab" href="#cptimp">
                  <img style="width: 30px" data-toggle="tooltip" title="Compteurs d'impulsion"
                       src="https://icons.abls-habitat.fr/assets/gif/Front_montant.png" />Compteurs d'impulsion</a></li>
            <li><a data-toggle="tab" href="#cpth">
                  <img style="width: 30px" data-toggle="tooltip" title="Compteurs horaire"
                       src="https://icons.abls-habitat.fr/assets/gif/Compteur_horaire.png" />Compteurs horaire</a></li>
            <li><a data-toggle="tab" href="#tempo">
                  <img style="width: 30px" data-toggle="tooltip" title="Temporisations"
                       src="https://icons.abls-habitat.fr/assets/gif/Sablier.png" />Tempos</a></li>
            <li><a data-toggle="tab" href="#horloge">
                  <img style="width: 30px" data-toggle="tooltip" title="Horloge"
                       src="https://icons.abls-habitat.fr/assets/gif/Calendar.png" />Horloges</a></li>
            <li><a data-toggle="tab" href="#messages">
                  <img style="width: 30px" data-toggle="tooltip" title="Messages"
                       src="https://icons.abls-habitat.fr/assets/gif/Message.png" />Messages</a></li>
          </ul>

          <div class="tab-content">
<!-----------------------------------------------Table des entrées TOR--------------------------------------------------------->
          <div id="entreetor" class="tab-pane fade in active" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Src Host</th>
                <th>Src Thread</th>
                <th>Src Text</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_DI as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Entrée TOR"
                         src="https://icons.abls-habitat.fr/assets/gif/Entree.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo htmlspecialchars($mnemo->libelle, ENT_QUOTES, 'UTF-8'); ?></td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/DI/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>
                  <td><select name="src_host" class="form-control">
                        <?php if ($mnemo->src_host=="*") $selected="selected"; else $selected='';
                              echo "<option value='*' ".$selected.">*</option>";
                              foreach ($instances as $instance):
                                if ($mnemo->src_host==$instance->instance_id) $selected="selected";else $selected='';
                                echo "<option value=".$instance->instance_id." ".$selected.">".$instance->instance_id."</option>";
                              endforeach;
                        ?>
                      </select>
                  </td>
                  <td><?php echo form_input( "src_thread", $mnemo->src_thread ); ?></td>
                  <td><?php echo form_input( "src_text", $mnemo->src_text ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/DI/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>

<!-----------------------------------------------Table des entrées ANA--------------------------------------------------------->
          <div id="entreeana" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Type</th>
                <th>Min/Max/Unité</th>
                <th>Map_host/thread/text</th>
                <th>Map_vocal</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_AI as $mnemo):?>
              <tr>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo anchor('archive/show/'.$mnemo->tech_id.'/'.$mnemo->acronyme.'/HOUR', $mnemo->libelle );?> </td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/AI/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>
                  <td><select name="type" class="form-control">
                        <?php $types = array( "Non Interprete", "4/20 mA 12 bits", "4/20 mA 10 bits", "WAGO 4/20 mA 750-455",
                                              "WAGO PT100 750-461" );
                              foreach ($types as $index => $type):
                                if ($mnemo->type==$index) $selected="selected";else $selected='';
                                echo "<option value=".$index." ".$selected.">".$type."</option>";
                              endforeach;
                        ?>
                      </select>
                  </td>
                  <td><?php echo form_input( "min", $mnemo->min ); ?>
                      <?php echo form_input( "max", $mnemo->max ); ?>
                      <?php echo form_input( "unite", $mnemo->unite ); ?></td>
                  <td><select name="map_host" class="form-control">
                        <?php if ($mnemo->map_host=="*") $selected="selected"; else $selected='';
                              echo "<option value='*' ".$selected.">*</option>";
                              foreach ($instances as $instance):
                                if ($mnemo->map_host==$instance->instance_id) $selected="selected";else $selected='';
                                echo "<option value=".$instance->instance_id." ".$selected.">".$instance->instance_id."</option>";
                              endforeach;
                        ?>
                      </select>
                      <?php echo form_input( "map_thread", $mnemo->map_thread ); ?>
                      <?php echo form_input( "map_text", $mnemo->map_text ); ?></td>
                  <td><?php echo form_input( "map_question_vocale", $mnemo->map_question_vocale ); ?>
                      <?php echo form_input( "map_reponse_vocale", $mnemo->map_reponse_vocale ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/AI/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>

<!-----------------------------------------------Table des sorties TOR--------------------------------------------------------->
          <div id="sortietor" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Dst Host</th>
                <th>Dst Thread</th>
                <th>Dst Tag</th>
                <th>Dst Param1</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_DO as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Sortie TOR"
                         src="https://icons.abls-habitat.fr/assets/gif/Sortie.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo htmlspecialchars($mnemo->libelle, ENT_QUOTES, 'UTF-8'); ?></td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/DO/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>
                  <td><select name="dst_host" class="form-control">
                        <?php if ($mnemo->dst_host=="*") $selected="selected"; else $selected='';
                              echo "<option value='*' ".$selected.">*</option>";
                              foreach ($instances as $instance):
                                if ($mnemo->dst_host==$instance->instance_id) $selected="selected";else $selected='';
                                echo "<option value=".$instance->instance_id." ".$selected.">".$instance->instance_id."</option>";
                              endforeach;
                        ?>
                      </select>
                  </td>
                  <td><?php echo form_input( "dst_thread", $mnemo->dst_thread ); ?></td>
                  <td><?php echo form_input( "dst_tag", $mnemo->dst_tag ); ?></td>
                  <td><?php echo form_input( "dst_param1", $mnemo->dst_param1 ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/DO/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>
<!-----------------------------------------------Table des sortie ANA---------------------------------------------------------->
          <div id="sortieana" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Type</th>
                <th>Min/Max</th>
                <th>Map_host/thread/text</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_AO as $mnemo):?>
              <tr>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo anchor('archive/show/'.$mnemo->tech_id.'/'.$mnemo->acronyme.'/HOUR', $mnemo->libelle );?> </td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/AO/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>
                  <td><select name="type" class="form-control">
                        <?php $types = array( "Non Interprete", "WAGO 4/20 mA 750-???", "WAGO 0-10Volts 750-???" );
                              foreach ($types as $index => $type):
                                if ($mnemo->type==$index) $selected="selected";else $selected='';
                                echo "<option value=".$index." ".$selected.">".$type."</option>";
                              endforeach;
                        ?>
                      </select>
                  </td>
                  <td><?php echo form_input( "min", $mnemo->min ); ?>
                      <?php echo form_input( "max", $mnemo->max ); ?>
                  <td><select name="map_host" class="form-control">
                        <?php if ($mnemo->map_host=="*") $selected="selected"; else $selected='';
                              echo "<option value='*' ".$selected.">*</option>";
                              foreach ($instances as $instance):
                                if ($mnemo->map_host==$instance->instance_id) $selected="selected";else $selected='';
                                echo "<option value=".$instance->instance_id." ".$selected.">".$instance->instance_id."</option>";
                              endforeach;
                        ?>
                      </select>
                      <?php echo form_input( "map_thread", $mnemo->map_thread ); ?>
                      <?php echo form_input( "map_text", $mnemo->map_text ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/AO/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>

<!-----------------------------------------------Table des registres ---------------------------------------------------------->
          <div id="registre" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Archivage</th>
                <th>Unite</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_R as $mnemo):?>
              <tr>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo anchor('archive/show/'.$mnemo->tech_id.'/'.$mnemo->acronyme.'/HOUR', $mnemo->libelle );?> </td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/R/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>
                  <td><?php echo form_checkbox( "archivage", 'archivage', $mnemo->archivage ); ?></td>
                  <td><?php echo form_input( "unite", $mnemo->unite ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/R/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>

<!-----------------------------------------------Table des compteurs d'impulsion ---------------------------------------------->
          <div id="cptimp" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Etat</th>
                <th>Valeur</th>
                <th>Multi</th>
                <th>Unite</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_CI as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Compteur d'impulsion"
                         src="https://icons.abls-habitat.fr/assets/gif/Front_montant.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo htmlspecialchars($mnemo->libelle, ENT_QUOTES, 'UTF-8'); ?></td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/CI/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>

                  <td><?php if ($mnemo->etat)
                       { echo '<span class="label label-success">Actif</span>'; }
                  else { echo '<span class="label label-default">Inactif</span>'; } ?></td>
                  <td><?php echo form_input( "valeur", $mnemo->valeur ); ?></td>
                  <td><?php echo form_input( "multi", $mnemo->multi ); ?></td>
                  <td><?php echo form_input( "unite", $mnemo->unite ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/CI/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>
<!-----------------------------------------------Table des compteurs horaire -------------------------------------------------->
          <div id="cpth" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Etat</th>
                <th>Valeur</th>
                <th>Appliquer</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_CH as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Compteur Horaire"
                         src="https://icons.abls-habitat.fr/assets/gif/Compteur_horaire.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo htmlspecialchars($mnemo->libelle, ENT_QUOTES, 'UTF-8'); ?></td>
                <form method="post" action=<?php echo site_url("/admin/mnemo/set/CH/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>

                  <td><?php if ($mnemo->etat)
                       { echo '<span class="label label-success">Actif</span>'; }
                  else { echo '<span class="label label-default">Inactif</span>'; } ?></td>
                  <td><?php echo form_input( "valeur", $mnemo->valeur ); ?></td>
                  <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
                <td><?php echo anchor("admin/mnemo/delete/CH/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>
<!-----------------------------------------------Table des Temporisations ----------------------------------------------------->
          <div id="tempo" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_TEMPO as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Temporisations"
                         src="https://icons.abls-habitat.fr/assets/gif/Sablier.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo htmlspecialchars($mnemo->libelle, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo anchor("admin/mnemo/delete/TEMPO/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>
<!-----------------------------------------------Table des Horloges ----------------------------------------------------------->
          <div id="horloge" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Type</th>
                <th>Acronyme</th>
                <th>Libellé</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($mnemos_HORLOGE as $mnemo):?>
              <tr>
                <td><img style="width: 30px" data-toggle="tooltip" title="Horloge"
                         src="https://icons.abls-habitat.fr/assets/gif/Calendar.png" /></td>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo anchor('horloges/show/'.$mnemo->tech_id.'/'.$mnemo->acronyme, $mnemo->libelle );?> </td>
                <td><?php echo anchor("admin/mnemo/delete/HORLOGE/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                               array('class' => 'btn btn-warning')); ?></td>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>

<!-----------------------------------------------Table des Messages ----------------------------------------------------------->
          <div id="messages" class="tab-pane fade in" style="padding:10px">
          <table class="table table-striped table-hover">
            <thead>
              <tr>
                <th>Acronyme</th>
                <th>TypeMSG</th>
                <th>Enable</th>
                <th>Libellé</th>
                <th>Audio</th>
                <th>Profil Audio</th>
                <th>Libellé Audio</th>
                <th>SMS</th>
                <th>Libellé SMS</th>
                <th>Modifier</th>
                <th>Supprimer</th>
              </tr>
            </thead>
            <tbody>
            <?php foreach ($msgs as $mnemo):?>
              <tr>
                <td><?php echo htmlspecialchars($mnemo->acronyme, ENT_QUOTES, 'UTF-8'); ?></td>
                <td><?php echo $this->Msg_model->type_to_icone($mnemo->type); ?></td>

                <form method="post" action=<?php echo site_url("/admin/mnemo/set/MSG/".$mnemo->tech_id."/".$mnemo->acronyme) ?>>

                <td><?php echo form_checkbox( "enable", 'enable', $mnemo->enable ); ?></td>
                <td><?php echo form_input( "libelle", $mnemo->libelle ); ?></td>

                <td><?php echo form_checkbox( "audio", 'audio', $mnemo->audio ); ?></td>
                <td> <select name="profil_audio" class="form-control">
                     <?php foreach ($profils_audio as $index => $profil):
                     if ($mnemo->profil_audio == $profil->acronyme) $selected="selected"; else $selected='';
                     echo "<option value=".$profil->acronyme." ".$selected.">".$profil->acronyme."</option>";
                     endforeach;
                     ?>
                     </select>
                </td>
                <td><?php echo form_input( "libelle_audio", $mnemo->libelle_audio ); ?></td>

                <td><select name="enable_sms" class="form-control">
                  <?php foreach ($this->msgs_types_sms as $index => $type_sms):
                        if ($mnemo->sms==$index) $selected="selected";else $selected='';
                        echo "<option value=".$index." ".$selected.">".$type_sms."</option>";
                        endforeach;
                  ?>
                    </select>
                </td>
                <td><?php echo form_input( "libelle_sms", $mnemo->libelle_sms ); ?></td>
                <td><?php echo form_submit( "submit", "Modifier", array('class' => 'btn btn-primary') )?></td>
                </form>
              <td><?php echo anchor("admin/mnemo/delete/MSG/".$mnemo->tech_id."/".$mnemo->acronyme, 'Supprimer',
                             array('class' => 'btn btn-warning')); ?>
              </tr>
            <?php endforeach;?>
            </tbody>
          </table>
          </div>
        </div>
<!--------------------------------------------------- Fin Tab ----------------------------------------------------------------->
        </div> <!-- box-->
      </div> <!-- col -->
    </div>  <!-- row-->
  </section>
</div>
