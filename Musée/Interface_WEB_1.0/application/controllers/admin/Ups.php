<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Ups extends Admin_Controller {

/******************************************************************************************************************************/
 function __construct()
  { parent::__construct();
    $this->load->model('Ups_model');
    $this->admin_page_title->push('Gérer les Onduleurs');
    $this->admin_breadcrumbs->unshift(1, 'Gestion des Onduleurs', 'admin/ups');
    $this->data['pagetitle'] = $this->admin_page_title->show();
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['onduleurs'] = $this->Ups_model->get_all();                                                    /* Get all ups */
    $this->template->admin_render('admin/ups/index', $this->data);                                           /* Load Template */
 	}
/******************************************************************************************************************************/
	function run()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['onduleurs'] = $this->wtd_webservice->get_primary("/process/ups/list");                        /* Get all ups */
    $this->template->admin_render('admin/ups/run', $this->data);                                             /* Load Template */
 	}
/******************************************************************************************************************************/
   public function activate($id)
    { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

      $id = (int) $id;
      $ups = $this->Ups_model->get($id);
      if (!isset($ups))
       { $this->session->set_flashdata('flash_error', 'Ups '.$id.' inconnu' );
         redirect('admin/ups');
       }
      $this->Ups_model->activate($id);
      $this->wtd_log->add("Wago/Ups ".$ups->id." (".$ups->ups."@".$ups->host.") activé");
      $this->wtd_webservice->send('/process/reload/ups');
      redirect('admin/ups');
    }
/******************************************************************************************************************************/
   public function deactivate($id = NULL)
    { if ( ! $this->wtd_auth->logged_in() ) { redirect('auth', 'refresh'); }

      $id = (int) $id;
      $ups = $this->Ups_model->get($id);
      if (!isset($ups))
       { $this->session->set_flashdata('flash_error', 'Wago/Ups '.$id.' inconnu' );
         redirect('admin/ups');
       }
      $this->Ups_model->deactivate($id);
      $this->wtd_log->add("Wago/Ups ".$ups->id." (".$ups->ups."@".$ups->host.") désactivé");
      $this->wtd_webservice->send('/process/reload/ups');
      redirect('admin/ups');
 	}

/******************************************************************************************************************************/
	public function create()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
           /* Breadcrumbs */
   	$data = array( 'enable'         => FALSE,
                   'tech_id'        => "NEW-".date("Ymd-His"),
                   'name'           => "ups_name",
                   'host'           => "host",
                   'username'       => "admin_user",
                   'password'       => "admin_password",
			                'libelle'        => "New UPS",
       		        );

  		if ($this->Ups_model->add($data))
     { $flash = "New UPS added";
       $this->wtd_log->add($flash);
       $this->session->set_flashdata('flash_message', $flash );
     }
    else
	    { $this->session->set_flashdata('flash_error', "Erreur lors de l'ajout Onduleur" );
       redirect('admin/ups');
		   }
            /* Load Template */
    redirect('admin/ups');
 	}
/******************************************************************************************************************************/
 public function delete($id)
  { $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $ups = $this->Ups_model->get($id);
    if (isset($ups))
     { $this->Ups_model->delete($id);
       $flash = "Onduleur ".$ups->ups."@".$ups->host.") supprimé";
       $this->session->set_flashdata('flash_message', $flash );
       $this->wtd_log->add( $flash );
       $this->wtd_webservice->send('/process/reload/ups');
     }
    else
     { $this->session->set_flashdata('flash_error', 'Tentative de suppression du ups '.$id.' inconnu' );
       $this->wtd_log->add("Onduleur ".$ups->id." inconnu ");
     }
    redirect('admin/ups');
  }

/******************************************************************************************************************************/
	public function edit($id=NULL)
	 {	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $this->data["ups"] = $ups = $this->Ups_model->get($id);
    if (!isset($ups))
     { $this->session->set_flashdata('flash_error', 'UPS inconnu' );
       redirect('admin/ups');
     }
        /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, "Edition de l'onduleur <strong>".$ups->tech_id, "</strong>admin/ups/edit");
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();


                		/* Validate form input */
		  $this->form_validation->set_rules('tech_id', 'Tech_ID', 'required');
    $this->form_validation->set_rules('name', 'Name', 'required');
    $this->form_validation->set_rules('host', 'Host', 'required');

		 	if ($this->form_validation->run() == TRUE)
		   {	$data = array( 'tech_id'        => strtoupper($this->input->post('tech_id')),
                      'enable'         => (null !== $this->input->post('enable') ? TRUE : FALSE),
                      'date_create'    => date("Y-m-d H:i:s"),
                      'name'           => $this->input->post('name'),
                      'host'           => $this->input->post('host'),
                      'username'       => $this->input->post('username'),
                      'password'       => $this->input->post('password'),
					                 'libelle'        => $this->input->post('libelle'),
         		        );

     		if ($this->Ups_model->update($ups->id, $data))
		      { $flash = "Onduleur <strong>".$ups->tech_id."</strong> (".$ups->name."@".$ups->host.") modifié.";
          $this->wtd_log->add($flash);
          $this->session->set_flashdata('flash_message', $flash );
			       $this->wtd_webservice->send('/process/reload/ups');
          redirect('admin/ups');
		      }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de mise à jour Onduleur'.$ups->tech_id );
          //redirect('admin/users', 'refresh');
        }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['username'] = array(	'name'  => 'username',
			                                  'id'    => 'username',
			                                  'type'  => 'text',
                                     'class' => 'form-control',
			                                  'value' => $this->form_validation->set_value('username', $ups->username)
		                                 );
    $this->data['password'] = array(	'name'  => 'password',
			                                  'id'    => 'password',
			                                  'type'  => 'text',
                                     'class' => 'form-control',
			                                  'value' => $this->form_validation->set_value('password', $ups->password)
		                                 );
    $this->data['name']     = array(	'name'  => 'name',
			                                  'id'    => 'name',
			                                  'type'  => 'text',
                                     'class' => 'form-control',
			                                  'value' => $this->form_validation->set_value('name', $ups->name)
		                                 );
    $this->data['host']     = array(	'name'  => 'host',
			                                  'id'    => 'host',
			                                  'type'  => 'text',
                                     'class' => 'form-control',
			                                  'value' => $this->form_validation->set_value('host', $ups->host)
		                                 );
    $this->data['tech_id']  = array(	'name'  => 'tech_id',
			                                  'id'    => 'tech_id',
			                                  'type'  => 'text',
                                     'class' => 'form-control',
			                                  'value' => $this->form_validation->set_value('tech_id', $ups->tech_id)
		                                 );
    $this->data['enable']   = array(	'name'  => 'enable',
			                                  'id'    => 'enable',
			                                  'checked' => $this->form_validation->set_value('enable', $ups->enable)
		                                 );
		  $this->data['libelle']  = array(	'name'  => 'libelle',
                                     'id'    => 'libelle',
                                     'type'  => 'text',
                                     'class' => 'form-control',
                                     'value' => $this->form_validation->set_value('libelle', $ups->libelle)
                                   );
		  $this->template->admin_render('admin/ups/edit', $this->data);
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
