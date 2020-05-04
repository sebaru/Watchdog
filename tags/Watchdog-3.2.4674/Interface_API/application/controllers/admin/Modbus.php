<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Modbus extends Admin_Controller {

/******************************************************************************************************************************/
 function __construct()
  { parent::__construct();
    $this->load->model('Modbus_model');
    $this->admin_page_title->push('Liste des Modules Wago/Modbus');
    $this->admin_breadcrumbs->unshift(1, 'Liste des Modules Wago/Modbus', 'admin/modbus');
    $this->data['pagetitle'] = $this->admin_page_title->show();
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['modbus'] = $this->Modbus_model->get_all()->result();                                    /* Get all modbus */
             
    $this->template->admin_render('admin/modbus/index', $this->data);                                        /* Load Template */
 	}
/******************************************************************************************************************************/
	function run()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['modbus'] = $this->wtd_webservice->get("/run/modbus/list");                                 /* Get all modbus */
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
      $this->wtd_webservice->send('/reload/modbus');
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
      $this->wtd_webservice->send('/reload/modbus');
      redirect('admin/modbus');
 	}

/******************************************************************************************************************************/
	public function create()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
           /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, lang('menu_users_create'), 'admin/users/create');
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
		
		  $this->data['access'] = $this->session->user_access_level;
    
  		/* Validate form input */
		  $this->form_validation->set_rules('sms_phone', 'Téléphone', 'required');
    $this->form_validation->set_rules('user_name', 'Login', 'required|is_unique[users.username]');
  		$this->form_validation->set_rules('email', 'E-Mail', 'required|valid_email');
  		$this->form_validation->set_rules('password', 'lang:users_password', 'required|min_length[' . $this->config->item('min_password_length').
                                                                         ']|max_length[' . $this->config->item('max_password_length') .
                                                                         ']|matches[password_confirm]');
  		$this->form_validation->set_rules('password_confirm', 'lang:users_password_confirm', 'required');

		 	if ($this->form_validation->run() == TRUE)
		   {	$data = array( 'username'       => $this->input->post('user_name'),
                      'enable'         => TRUE,
                      'date_create'    => date("Y-m-d H:i:s"),
                      'access_level'   => $this->input->post('access_level'),
                      'sms_phone'      => $this->input->post('sms_phone'),
					                 'comment'        => $this->input->post('comment'),
					                 'email'          => strtolower($this->input->post('email')),
					                 'imsg_enable'    => $this->input->post('imsg_enable') !== NULL ? TRUE : FALSE,
					                 'imsg_jabberid'  => $this->input->post('imsg_jabberid'),
					                 'imsg_allow_cde' => $this->input->post('imsg_allow_cde') !== NULL ? TRUE : FALSE,
					                 'password'       => $this->input->post('password')
         		        );

     		if ($this->wtd_auth_model->register($data))
		      { $this->session->set_flashdata('flash_message', 'Utilisateur '.$this->input->post('user_name').' ajouté' );
			       redirect('admin/users', 'refresh');
		      }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de mise à jour utilisateur' );
       						//redirect('admin/users', 'refresh');
        }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['user_name'] = array(	'name'  => 'user_name',
			                                   'id'    => 'user_name',
			                                   'type'  => 'text',
                                      'class' => 'form-control',
			                                   'value' => $this->form_validation->set_value('username', $this->input->post('user_name'))
		                                  );
		  $this->data['comment'] = array(	'name'  => 'comment',
                                    'id'    => 'comment',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('comment', $this->input->post('comment'))
                                  );
  		$this->data['access_level'] = array(	'name'  => 'access_level',
                                         'id'    => 'access_level',
                                         'type'  => 'text',
                                         'class' => 'form-control',
                                         'value' => $this->form_validation->set_value('access_level', $this->input->post('access_level'))
                                       );
		  $this->data['sms_phone'] = array( 'name'  => 'sms_phone',
                                      'id'    => 'sms_phone',
                                      'type'  => 'tel',
                                      'pattern' => '^((\+\d{1,3}(-| )?\(?\d\)?(-| )?\d{1,5})|(\(?\d{2,6}\)?))(-| )?(\d{3,4})(-| )?(\d{4})(( x| ext)\d{1,5}){0,1}$',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('sms_phone', $this->input->post('sms_phone'))
                               		  );
    $this->data['imsg_enable'] = array(	'name'  => 'imsg_enable',
                                        'id'    => 'imsg_enable',
                                        'checked' => $this->form_validation->set_value('imsg_enable', $this->input->post('imsg_enable'))
                                      );
		  $this->data['imsg_allow_cde'] = array(	'name'  => 'imsg_allow_cde',
                                           'id'    => 'imsg_allow_cde',
                                           'value' => 'imsg_allow_cde',
                                           'checked' => $this->form_validation->set_value('imsg_allow_cde', $this->input->post('imsg_allow_cde'))
                                         );
		  $this->data['imsg_jabberid'] = array(	'name'  => 'imsg_jabberid',
                                          'id'    => 'imsg_jabberid',
                                          'type'  => 'email',
                                          'class' => 'form-control',
                                          'value' => $this->form_validation->set_value('imsg_jabberid', $this->input->post('imsg_jabberid'))
                                        );
		  $this->data['email'] = array(	'name'  => 'email',
                                  'id'    => 'email',
                                  'type'  => 'email',
                                  'class' => 'form-control',
                                  'value' => $this->form_validation->set_value('email')
                                );
    $this->data['password'] = array(	'name' => 'password',
                                     'id'   => 'password',
                                     'class' => 'form-control',
                                     'type' => 'password'
                                   );
    $this->data['password_confirm'] = array( 'name' => 'password_confirm',
                                             'id'   => 'password_confirm',
                                             'class' => 'form-control',
                                             'type' => 'password'
		                                         );

            /* Load Template */
    $this->template->admin_render('admin/users/create', $this->data);
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
       $this->wtd_webservice->send('/reload/modbus');
     }
    else
     { $this->session->set_flashdata('flash_error', 'Tentative de suppression du modbus '.$id.' inconnu' );
       $this->wtd_log->add("Wago/Modbus ".$target_modbus->id." inconnu ");
     }
    redirect('admin/modbus');
  }

/******************************************************************************************************************************/
	public function edit($id)
	 {	$id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
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
                 			);

       if($this->Modbus_model->update($id, $data))
 			    { $flash='Module '.$this->input->post('hostname').' ('.$this->input->post('tech_id').') modifié';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/reload/modbus');
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
