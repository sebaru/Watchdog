<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemo extends Admin_Controller {

 public function __construct()
  { parent::__construct();
    $this->load->model('Config_model');
    $this->load->model('Mnemo_model');
    $this->load->model('Dls_model');
    $this->load->model('Msg_model');

        /* Title Page :: Common */
    $this->admin_page_title->push('Mnémoniques');
 			$this->admin_breadcrumbs->unshift(1, 'Modules D.L.S', 'admin/dls');
    $this->data['pagetitle'] = $this->admin_page_title->show();

    $this->msgs_types = array
     ( '<img style="width: 20px" data-toggle="tooltip" title="Information" src="https://icons.abls-habitat.fr/assets/gif/Info.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Alerte"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Défaut"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Alarme"      src="https://icons.abls-habitat.fr/assets/gif/Pignon_orange.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Veille"      src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_vert.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Attente"     src="https://icons.abls-habitat.fr/assets/gif/Bouclier2_noir.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Danger"      src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Dérangement" src="https://icons.abls-habitat.fr/assets/gif/Croix_rouge_jaune.svg" />',
     );
    $this->msgs_types_sms   = array ( 'Désactivé', 'Activé', 'GSM Only', 'SMSBOXOnly' );
  }

/******************************************************************************************************************************/
	public function index($tech_id=null)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
 			$this->admin_breadcrumbs->unshift(2, 'Liste des mnémoniques', 'admin/mnemo/index/'.$tech_id);
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */

    $this->data['instances'] = $this->Config_model->get_instances();                                         /* Get all users */
    $this->data['dls'] = $this->Dls_model->get($tech_id);
    $this->data['mnemos_DI'] = $this->Mnemo_model->get_all_DI($tech_id);                                     /* Get all users */
    $this->data['mnemos_DO'] = $this->Mnemo_model->get_all_DO($tech_id);                                     /* Get all users */
    $this->data['mnemos_CI'] = $this->Mnemo_model->get_all_CI($tech_id);                                     /* Get all users */
    foreach ($this->data['mnemos_CI'] as $mnemo):
      $result= $this->wtd_webservice->post( '/memory',
               array ( "mode" => "GET", "type" =>"CI",
                       "tech_id" => $mnemo->tech_id, "acronyme" => $mnemo->acronyme ) );
      if ($result)
       { $mnemo->etat   = $result->etat;
         $mnemo->valeur   = $result->valeur;
       }
    endforeach;
    $this->data['mnemos_CH'] = $this->Mnemo_model->get_all_CH($tech_id);                                     /* Get all users */
    foreach ($this->data['mnemos_CH'] as $mnemo):
      $result= $this->wtd_webservice->post( '/memory',
               array ( "mode" => "GET", "type" =>"CH",
                       "tech_id" => $mnemo->tech_id, "acronyme" => $mnemo->acronyme ) );
      if ($result)
       { $mnemo->etat   = $result->etat;
         $mnemo->valeur   = $result->valeur;
       }
    endforeach;
    $this->data['mnemos_AI'] = $this->Mnemo_model->get_all_AI($tech_id);                                     /* Get all users */
    $this->data['mnemos_AO'] = $this->Mnemo_model->get_all_AO($tech_id);                                     /* Get all users */
    $this->data['mnemos_R']  = $this->Mnemo_model->get_all_R($tech_id);                                      /* Get all users */
    $this->data['mnemos_HORLOGE'] = $this->Mnemo_model->get_all_HORLOGE($tech_id);                           /* Get all users */
    $this->data['mnemos_TEMPO'] = $this->Mnemo_model->get_all_TEMPO($tech_id);                               /* Get all users */
    $this->data['msgs'] = $this->Msg_model->get_all_MSG($tech_id);
    $this->data['profils_audio'] = $this->Msg_model->get_all_profils_audio();
    $this->template->admin_render('admin/mnemo/index', $this->data);                                         /* Load Template */
 	}
/******************************************************************************************************************************/
	public function delete($type,$tech_id,$acronyme)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    switch($type)
     { case "DI":      $mnemo = $this->Mnemo_model->get_DI($tech_id,$acronyme); break;
       case "DO":      $mnemo = $this->Mnemo_model->get_DO($tech_id,$acronyme); break;
       case "MSG":     $mnemo = $this->Msg_model->get_MSG($tech_id,$acronyme); break;
       case "CI":      $mnemo = $this->Mnemo_model->get_CI($tech_id,$acronyme); break;
       case "AI":      $mnemo = $this->Mnemo_model->get_AI($tech_id,$acronyme); break;
       case "AO":      $mnemo = $this->Mnemo_model->get_AO($tech_id,$acronyme); break;
       case "CH":      $mnemo = $this->Mnemo_model->get_CH($tech_id,$acronyme); break;
       case "R" :      $mnemo = $this->Mnemo_model->get_R($tech_id,$acronyme); break;
       case "HORLOGE": $mnemo = $this->Mnemo_model->get_HORLOGE($tech_id,$acronyme); break;
       case "TEMPO":   $mnemo = $this->Mnemo_model->get_TEMPO($tech_id,$acronyme); break;
     }

    if (!isset($mnemo))
     { $this->session->set_flashdata('flash_error', 'Mnémonique inconnu' );
       redirect('admin/mnemo/index/1');
     }
    if ($mnemo->access_level < $this->session->user_access_level)
     { switch($type)
        { case "DI": $retour = $this->Mnemo_model->delete_DI($mnemo->tech_id,$mnemo->acronyme); break;
          case "DO": $retour = $this->Mnemo_model->delete_DO($mnemo->tech_id,$mnemo->acronyme); break;
          case "CI": $retour = $this->Mnemo_model->delete_CI($mnemo->tech_id,$mnemo->acronyme); break;
          case "AI": $retour = $this->Mnemo_model->delete_AI($mnemo->tech_id,$mnemo->acronyme); break;
          case "AO": $retour = $this->Mnemo_model->delete_AO($mnemo->tech_id,$mnemo->acronyme); break;
          case "CH": $retour = $this->Mnemo_model->delete_CH($mnemo->tech_id,$mnemo->acronyme); break;
          case "R":  $retour = $this->Mnemo_model->delete_R ($mnemo->tech_id,$mnemo->acronyme); break;
          case "HORLOGE": $retour = $this->Mnemo_model->delete_HORLOGE($mnemo->tech_id,$mnemo->acronyme); break;
          case "TEMPO":   $retour = $this->Mnemo_model->delete_TEMPO($mnemo->tech_id,$mnemo->acronyme); break;
          case "MSG": $retour = $this->Msg_model->delete_MSG($mnemo->id); break;
        }
       if($retour)
 			    { $flash = 'Mnémonique '.$mnemo->tech_id.":".$mnemo->acronyme.' supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression mnémonique' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Permission error' );
       $this->wtd_log->add("Tentative de suppression du mnémonique ".$mnemo->tech_id.":".$mnemo->acronyme);
     }
    redirect('admin/mnemo/index/'.$mnemo->tech_id);
	 }
/******************************************************************************************************************************/
	public function set($type,$tech_id,$acronyme)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    switch($type)
     { case "DI":      $mnemo = $this->Mnemo_model->get_DI($tech_id,$acronyme); break;
       case "DO":      $mnemo = $this->Mnemo_model->get_DO($tech_id,$acronyme); break;
       case "MSG":     $mnemo = $this->Msg_model->get_MSG($tech_id,$acronyme); break;
       case "CI":      $mnemo = $this->Mnemo_model->get_CI($tech_id,$acronyme); break;
       case "AI":      $mnemo = $this->Mnemo_model->get_AI($tech_id,$acronyme); break;
       case "AO":      $mnemo = $this->Mnemo_model->get_AO($tech_id,$acronyme); break;
       case "CH":      $mnemo = $this->Mnemo_model->get_CH($tech_id,$acronyme); break;
       case "R":       $mnemo = $this->Mnemo_model->get_R($tech_id,$acronyme); break;
       case "HORLOGE": $mnemo = $this->Mnemo_model->get_HORLOGE($tech_id,$acronyme); break;
     }

    if (!isset($mnemo))
     { $this->session->set_flashdata('flash_error', 'Mnémonique inconnu' );
       redirect('admin/mnemo/index/1');
     }

    if ($mnemo->access_level < $this->session->user_access_level)
     { switch($type)
        { case "DI": $retour = $this->Mnemo_model->set_DI($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "src_host"   => $this->input->post("src_host"),
                                              "src_thread" => strtoupper($this->input->post("src_thread")),
                                              "src_text"   => strtoupper($this->input->post("src_text")) ) );
                     $this->wtd_webservice->send('/process/reload/'.$mnemo->src_thread);
                     $new_thread = strtoupper($this->input->post("src_thread"));
                     if (strcmp($mnemo->src_thread, $new_thread))
                      { $this->wtd_webservice->send('/process/reload/'.$new_thread); }
                     break;
          case "DO": $retour = $this->Mnemo_model->set_DO($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "dst_host"   => $this->input->post("dst_host"),
                                              "dst_thread" => strtoupper($this->input->post("dst_thread")),
                                              "dst_tag"    => strtoupper($this->input->post("dst_tag")),
                                              "dst_param1" => $this->input->post("dst_param1") ) );
                     $this->wtd_webservice->send('/process/reload/'.$mnemo->dst_thread);
                     $new_thread = strtoupper($this->input->post("dst_thread"));
                     if (strcmp($mnemo->dst_thread, $new_thread))
                      { $this->wtd_webservice->send('/process/reload/'.$new_thread); }
                     break;
          case "CI": $retour = $this->Mnemo_model->set_CI($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "unite"  => $this->input->post("unite"),
                                              "multi"  => $this->input->post("multi"),
                                              "valeur" => $this->input->post("valeur") ) );
                     $this->wtd_webservice->post( '/memory',
                            array ( "mode" => "SET", "type" =>"CI",
                                    "tech_id" => $mnemo->tech_id, "acronyme" => $mnemo->acronyme,
                                    "unite"  => $this->input->post("unite"),
                                    "multi"  => $this->input->post("multi"),
                                    "valeur" => $this->input->post("valeur") ) );
                     break;
          case "CH": $retour=TRUE;
                     $this->wtd_webservice->post( '/memory',
                            array ( "mode" => "SET", "type" =>"CH",
                                    "tech_id" => $mnemo->tech_id, "acronyme" => $mnemo->acronyme,
                                    "valeur" => $this->input->post("valeur") ) );
                     break;
          case "R":  $retour = $this->Mnemo_model->set_R($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "unite"  => $this->input->post("unite"),
                                              "archivage"  => (null !== $this->input->post("archivage"))) );
                     $this->wtd_webservice->post( '/memory',
                            array ( "mode" => "SET", "type" =>"R",
                                    "tech_id" => $mnemo->tech_id, "acronyme" => $mnemo->acronyme,
                                    "unite"  => $this->input->post("unite"),
                                    "archivage"  => (null !== $this->input->post("archivage")) ) );
                     break;
          case "HORLOGE":
                     $retour = $this->Mnemo_model->set_HORLOGE($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "heure"  => $this->input->post("heure"),
                                              "minute" => $this->input->post("minute"),
                                              "lundi"  => $this->input->post("lundi"),
                                            ) );
                     break;
          case "AI": $retour = $this->Mnemo_model->set_AI($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "map_host"   => $this->input->post("map_host"),
                                              "map_thread" => strtoupper($this->input->post("map_thread")),
                                              "map_text"   => strtoupper($this->input->post("map_text")),
                                              "unite"  => $this->input->post("unite"),
                                              "min"    => $this->input->post("min"),
                                              "max"    => $this->input->post("max"),
                                              "type"   => $this->input->post("type"),
                                            ) );
                     $this->wtd_webservice->send('/process/reload/'.$mnemo->map_thread);
                     $new_thread = strtoupper($this->input->post("map_thread"));
                     if (strcmp($mnemo->map_thread, $new_thread))
                      { $this->wtd_webservice->send('/process/reload/'.$new_thread); }
                     break;
          case "AO": $retour = $this->Mnemo_model->set_AO($mnemo->tech_id, $mnemo->acronyme,
                                      array ( "map_host"   => $this->input->post("map_host"),
                                              "map_thread" => strtoupper($this->input->post("map_thread")),
                                              "map_text"   => strtoupper($this->input->post("map_text")),
                                              "min"    => $this->input->post("min"),
                                              "max"    => $this->input->post("max"),
                                              "type"   => $this->input->post("type"),
                                            ) );
                     $this->wtd_webservice->send('/process/reload/'.$mnemo->map_thread);
                     $new_thread = strtoupper($this->input->post("map_thread"));
                     if (strcmp($mnemo->map_thread, $new_thread))
                      { $this->wtd_webservice->send('/process/reload/'.$new_thread); }
                     break;
          case "MSG":$retour = $this->Msg_model->set_MSG($mnemo->id,
                                      array ( "enable"   => (null !== $this->input->post("enable")),
                                              "libelle"  => $this->input->post("libelle"),
                                              "audio"    => (null !== $this->input->post("audio")),
                                              "profil_audio"   => $this->input->post("profil_audio"),
                                              "libelle_audio"  => $this->input->post("libelle_audio"),
                                              "sms"      => $this->input->post("enable_sms"),
                                              "libelle_sms"  => $this->input->post("libelle_sms"),
                                            ) );
                     break;
        }
       if($retour)
 			    { $flash = 'Mnémonique '.$mnemo->tech_id.":".$mnemo->acronyme.' modifié';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de modification de mnémonique' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Permission error' );
       $this->wtd_log->add("Tentative de modification du mnémonique ".$mnemo->tech_id.":".$mnemo->acronyme);
     }
    redirect('admin/mnemo/index/'.$mnemo->tech_id);
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
