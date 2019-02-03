<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemo extends Admin_Controller {

 public function __construct()
  { parent::__construct();
    $this->load->model('Mnemo_model');

        /* Title Page :: Common */
    $this->admin_page_title->push('Mnémoniques');
 			$this->admin_breadcrumbs->unshift(1, 'Modules D.L.S', 'admin/dls');
    $this->data['pagetitle'] = $this->admin_page_title->show();

   /* $this->mnemos_types = array
     ( '<img style="width: 20px" data-toggle="tooltip" title="Bistable"           src="https://icons.abls-habitat.fr/Info.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Monostable"         src="https://icons.abls-habitat.fr/Pignon_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Temporisation"      src="https://icons.abls-habitat.fr/Pignon_orange.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Entrée TOR"         src="https://icons.abls-habitat.fr/Pignon_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="sortie TOR"         src="https://icons.abls-habitat.fr/Bouclier2_vert.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Entrée ANA"         src="https://icons.abls-habitat.fr/Bouclier2_noir.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Sortie ANA"         src="https://icons.abls-habitat.fr/Croix_rouge_rouge.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Visuel"             src="https://icons.abls-habitat.fr/Croix_rouge_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Compteur horaire"   src="https://icons.abls-habitat.fr/Croix_rouge_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Compteur Impulsion" src="https://icons.abls-habitat.fr/Croix_rouge_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Registre"           src="https://icons.abls-habitat.fr/Croix_rouge_jaune.svg" />',
       '<img style="width: 20px" data-toggle="tooltip" title="Horloge"            src="https://icons.abls-habitat.fr/Horloge_neutre.svg" />',
     );*/
        /* Breadcrumbs :: Common */
  }

/******************************************************************************************************************************/
	public function index($dls_id=null)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
 			$this->admin_breadcrumbs->unshift(2, 'Liste des mnémoniques', 'admin/mnemo/index/'.$dls_id);
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
    $this->data['mnemos'] = $this->Mnemo_model->get_all($dls_id);                                            /* Get all users */

    $this->template->admin_render('admin/mnemo/index', $this->data);                                         /* Load Template */
 	}
/******************************************************************************************************************************/
	public function delete($id)
 	{ $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_mnemo = $this->Mnemo_model->get($id);
    if (!isset($target_mnemo))
     { $this->session->set_flashdata('flash_error', 'Mnémonique inconnu' );
       redirect('admin/dls/index');
     }
    if ($target_mnemo->access_level < $this->session->user_access_level)
     { if($this->Mnemo_model->delete($target_mnemo->id))
 			    { $this->session->set_flashdata('flash_message', 'Mnémonique supprimé' );
          $this->wtd_log->add("Mnémonique ".$target_mnemo->id." supprimé");
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression mnémonique' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Permission error' );
       $this->wtd_log->add("Tentative de suppression du mnémonique ".$target_mnemo->id);
     }
    redirect('admin/mnemo/index/'.$target_mnemo->dls_id);
	 }
/******************************************************************************************************************************/
	public function edit($id)
	 {	$id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		  if (!isset($id)) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

			 // check if the mnemo exists before trying to edit it
			 $this->data['mnemo'] = $mnemo = $this->Mnemo_model->get($id);
 			if(!isset($mnemo))
     { $this->session->set_flashdata('flash_error', "Ce mnémonique n'existe pas");
       redirect('admin/dls/index');
     }

 			if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }

        /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, "Edition du mnémonique", 'admin/mnemo/edit/'.$id );
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
		
                		/* Validate form input */
    $this->form_validation->set_rules('libelle', 'Libellé', 'required');
 			if ($this->form_validation->run() == TRUE)
     {	$data = array( 'libelle'   => $this->input->post('libelle'),
		                    'ev_host'   => $this->input->post('ev_host'),
                      'ev_thread' => strtoupper($this->input->post('ev_thread')),
					                 'ev_text'   => $this->input->post('ev_text'),
                 			);

       if($this->Mnemo_model->update($mnemo->id, $data))
 			    { $flash='Le mnémonique <strong>'.$mnemo->tech_id.':'.$mnemo->acronyme.'</strong> ('.$mnemo->id.') a été modifié'; 
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add( $flash );
          if($mnemo->ev_thread=="VOICE"  || $data['ev_thread']=="VOICE")  { $this->wtd_webservice->send('/reload/voice'); }
          if($mnemo->ev_thread=="MODBUS" || $data['ev_thread']=="MODBUS") { $this->wtd_webservice->send('/reload/modbus'); }
          redirect('admin/mnemo/index/'.$mnemo->dls_id);
 			    }
       else
 			    { $flash='Tentative de modification du mnémonique '.$mnemo->tech_id.':'.$mnemo->acronyme.'('.$mnemo->id.')'; 
          $this->session->set_flashdata('flash_error', $flash );
          $this->wtd_log->add( $flash );
          redirect('admin/mnemo/index/'.$mnemo->dls_id);
 			    }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

		  $this->data['ppage'] = array(	'name'  => 'ppage',
	                                 'id'    => 'ppage',
	                                 'type'  => 'text',
                                  'class' => 'form-control',
                                  'disabled' => 'TRUE',
	                                 'value' => $this->form_validation->set_value('ppage', $mnemo->ppage)
                                );
		  $this->data['page'] = array(	'name'  => 'page',
	                                'id'    => 'page',
	                                'type'  => 'text',
                                 'class' => 'form-control',
                                 'disabled' => 'TRUE',
	                                'value' => $this->form_validation->set_value('page', $mnemo->page)
                               );
		  $this->data['dls_shortname'] = array(	'name'  => 'dls_shortname',
	                                         'id'    => 'dls_shortname',
	                                         'type'  => 'text',
                                          'class' => 'form-control',
                                          'disabled' => 'TRUE',
	                                         'value' => $this->form_validation->set_value('dls_shortname', $mnemo->shortname)
                                        );
		  $this->data['dls_tech_id'] = array(	'name'  => 'dls_tech_id',
	                                       'id'    => 'dls_tech_id',
	                                       'type'  => 'text',
                                        'class' => 'form-control',
                                        'disabled' => 'TRUE',
	                                       'value' => $this->form_validation->set_value('dls_tech_id', $mnemo->tech_id)
                                      );
		  $this->data['acronyme'] = array(	'name'  => 'acronyme',
                                     'id'    => 'acronyme',
                                     'type'  => 'text',
                                     'class' => 'form-control',
                                     'disabled' => 'TRUE',
                                     'value' => $this->form_validation->set_value('acronyme', $mnemo->acronyme)
                                   );
		  $this->data['libelle'] = array(	'name'  => 'libelle',
	                                   'id'    => 'libelle',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('libelle', $mnemo->libelle)
                                  );
		  $this->data['type'] = array(	'name'  => 'type',
	                                'id'    => 'type',
	                                'type'  => 'text',
                                 'class' => 'form-control',
                                 'disabled' => 'TRUE',
	                                'value' => $this->form_validation->set_value('type', $mnemo->type)
                               );
		  $this->data['ev_host'] = array(	'name'  => 'ev_host',
	                                   'id'    => 'ev_host',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('ev_host', $mnemo->ev_host)
                                  );
		  $this->data['ev_thread'] = array(	'name'  => 'ev_thread',
	                                     'id'    => 'ev_thread',
	                                     'type'  => 'text',
                                      'class' => 'form-control',
                                      /*'disabled' => 'TRUE',*/
	                                     'value' => $this->form_validation->set_value('ev_thread', $mnemo->ev_thread)
                                    );
		  $this->data['ev_text'] = array(	'name'  => 'ev_text',
	                                   'id'    => 'ev_text',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('ev_text', $mnemo->ev_text)
                                  );
		  $this->data['tableau'] = array(	'name'  => 'tableau',
	                                   'id'    => 'tableau',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
                                    /*'disabled' => 'TRUE',*/
	                                   'value' => $this->form_validation->set_value('tableau', $mnemo->tableau)
                                  );
        /* Load Template */
		  $this->template->admin_render('admin/mnemo/edit', $this->data);
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
