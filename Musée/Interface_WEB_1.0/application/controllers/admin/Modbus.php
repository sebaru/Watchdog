<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Modbus extends Admin_Controller {

/******************************************************************************************************************************/
 function __construct()
  { parent::__construct();
    $this->load->model('Modbus_model');
    $this->load->model('Mnemo_model');
    $this->admin_page_title->push('Liste des Modules Wago/Modbus');
    $this->admin_breadcrumbs->unshift(1, 'Liste des Modules Wago/Modbus', 'admin/modbus');
    $this->data['pagetitle'] = $this->admin_page_title->show();
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['modbus'] = $this->Modbus_model->get_all()->result();                                       /* Get all modbus */
    $this->template->admin_render('admin/modbus/index', $this->data);                                        /* Load Template */
 	}
/******************************************************************************************************************************/
	function run()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['modbus'] = $this->wtd_webservice->get_primary("/process/modbus/list");                     /* Get all modbus */
    $this->template->admin_render('admin/modbus/run', $this->data);                                          /* Load Template */
 	}
/******************************************************************************************************************************/
   public function activate($id)
    { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

      $id = (int) $id;
      $target_modbus = $this->Modbus_model->get($id);
      if (!isset($target_modbus))
       { $this->session->set_flashdata('flash_error', 'Wago/Modbus '.$id.' inconnu' );
         redirect('admin/modbus');
       }
      $this->Modbus_model->activate($id);
      $this->wtd_log->add("Wago/Modbus ".$target_modbus->id." (".$target_modbus->hostname.") activé");
      $this->wtd_webservice->send('/process/reload/modbus');
      redirect('admin/modbus');
    }
/******************************************************************************************************************************/
   public function deactivate($id = NULL)
    { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

      $id = (int) $id;
      $target_modbus = $this->Modbus_model->get($id);
      if (!isset($target_modbus))
       { $this->session->set_flashdata('flash_error', 'Wago/Modbus '.$id.' inconnu' );
         redirect('admin/modbus');
       }
      $this->Modbus_model->deactivate($id);
      $this->wtd_log->add("Wago/Modbus ".$target_modbus->id." (".$target_modbus->hostname.") désactivé");
      $this->wtd_webservice->send('/process/reload/modbus');
      redirect('admin/modbus');
 	}

/******************************************************************************************************************************/
	public function create()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
   	$data = array( 'enable'         => FALSE,
                   'tech_id'        => "NEW-".date("Ymd-His"),
                   'date_create'    => "NOW()",
                   'watchdog'       => "200",
			                'description'    => "New WAGO",
       		        );

  		if ($this->Modbus_model->add($data))
     { $flash = "New Wago added";
       $this->wtd_log->add($flash);
       $this->session->set_flashdata('flash_message', $flash );
     }
    else
	    { $this->session->set_flashdata('flash_error', "Erreur lors de l'ajout Onduleur" );
       redirect('admin/modbus');
		   }
            /* Load Template */
    redirect('admin/modbus');
 	}
/******************************************************************************************************************************/
 public function delete($id)
  { $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_modbus = $this->Modbus_model->get($id);
    if (isset($target_modbus))
     { $this->Modbus_model->delete($id);
       $flash = 'Wago/Modbus '.$target_modbus->hostname.' ('.$target_module->tech_id.') supprimé';
       $this->session->set_flashdata('flash_message', $flash );
       $this->wtd_log->add( $flash );
       $this->wtd_webservice->send('/process/reload/modbus');
     }
    else
     { $this->session->set_flashdata('flash_error', 'Tentative de suppression du modbus '.$id.' inconnu' );
       $this->wtd_log->add("Wago/Modbus ".$target_modbus->id." inconnu ");
     }
    redirect('admin/modbus');
  }

/******************************************************************************************************************************/
	public function edit($id=NULL)
	 {	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

        /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, 'Edition du module '.$id, 'admin/modbus/edit');
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

                		/* Validate form input */
    $this->form_validation->set_rules('hostname', 'Hostname', 'required');
    $this->form_validation->set_rules('tech_id', 'tech_id', 'required');
 			if ($this->form_validation->run() == TRUE)
     {	$data = array( 'hostname'  => $this->input->post('hostname'),
                      'tech_id'   => $this->input->post('tech_id'),
                      'description' => $this->input->post('description'),
		                    'map_E'     => $this->input->post('map_E'),
		                    'max_nbr_E' => $this->input->post('max_nbr_E'),
		                    'map_EA'    => $this->input->post('map_EA'),
		                    'map_A'     => $this->input->post('map_A'),
		                    'map_AA'    => $this->input->post('map_AA'),
		                    'watchdog'  => $this->input->post('watchdog'),
                 			);

       if($this->Modbus_model->update($id, $data))
 			    { $flash='Module '.$this->input->post('hostname').' ('.$this->input->post('tech_id').') modifié';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/process/reload/modbus');
          $this->wtd_log->add( $flash );
    						//redirect('admin/users', 'refresh');
 			    }
       else
 			    { $flash='Erreur de mise à jour du module ' . $this->input->post('hostname').' ('.$this->input->post('tech_id').')';
 			      $this->session->set_flashdata('flash_error', $flash );
          $this->wtd_log->add( $flash );
    						//redirect('admin/users', 'refresh');
        }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $modbus = $this->Modbus_model->get($id);
    $this->data['maps_di'] = $this->Mnemo_model->get_all_DI_by_thread($modbus->tech_id,'MODBUS');           /* Get all modbus */
    $this->data['maps_ai'] = $this->Mnemo_model->get_all_AI_by_thread($modbus->tech_id,'MODBUS');           /* Get all modbus */
		  $this->data['hostname'] = array(	'name'  => 'hostname',
		                                   'id'    => 'hostname',
		                                   'type'  => 'text',
                                     'class' => 'form-control',
		                                   'value' => $this->form_validation->set_value('hostname', $modbus->hostname)
	                                  );
		  $this->data['tech_id']  = array(	'name'  => 'tech_id',
                                     'id'    => 'tech_id',
	                                    'type'  => 'text',
                                     'class' => 'form-control',
                                     'value' => $this->form_validation->set_value('tech_id', $modbus->tech_id)
                                   );
		  $this->data['watchdog']  = array(	'name'  => 'watchdog',
                                      'id'    => 'watchdog',
	                                     'type'  => 'text',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('watchdog', $modbus->watchdog)
                                    );
		  $this->data['description']  = array(	'name'  => 'description',
		                                       'id'    => 'description',
   		                                    'type'  => 'text',
                                         'class' => 'form-control',
		                                       'value' => $this->form_validation->set_value('description', $modbus->description)
	                                      );

		  $this->data['map_E'] = array(	'name'  => 'map_E',
		                                'id'    => 'map_E',
                                  'type'  => 'number',
                                  'class' => 'form-control',
		                                'value' => $this->form_validation->set_value('map_E', $modbus->map_E)
	                                  );
		  $this->data['max_nbr_E'] = array(	'name'  => 'max_nbr_E',
		                                    'id'    => 'max_nbr_E',
                                      'type'  => 'number',
                                      'class' => 'form-control',
		                                    'value' => $this->form_validation->set_value('map_E', $modbus->max_nbr_E)
                                    );
		  $this->data['map_EA'] = array(	'name'  => 'map_EA',
		                                'id'    => 'map_EA',
                                  'type'  => 'number',
                                  'class' => 'form-control',
		                                'value' => $this->form_validation->set_value('map_EA', $modbus->map_EA)
                                 );
		  $this->data['map_A'] = array(	'name'  => 'map_A',
		                                'id'    => 'map_A',
                                  'type'  => 'number',
                                  'class' => 'form-control',
		                                'value' => $this->form_validation->set_value('map_A', $modbus->map_A)
                                );
		  $this->data['map_AA'] = array(	'name'  => 'map_AA',
		                                'id'    => 'map_AA',
                                  'type'  => 'number',
                                  'class' => 'form-control',
		                                'value' => $this->form_validation->set_value('map_AA', $modbus->map_AA)
                                 );
        /* Load Template */
		  $this->template->admin_render('admin/modbus/edit', $this->data);
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
