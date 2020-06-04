<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Users extends Admin_Controller {

 public function __construct()
  { parent::__construct();
        /* Load :: Common */
    $this->lang->load('admin/users');

        /* Title Page :: Common */
    $this->admin_page_title->push(lang('menu_users'));
    $this->data['pagetitle'] = $this->admin_page_title->show();

        /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Liste des utilisateurs', 'admin/users');
  }

/******************************************************************************************************************************/
	public function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
    $this->data['users'] = $this->wtd_auth_model->user_list()->result();                                     /* Get all users */

    $this->template->admin_render('admin/users/index', $this->data);                                         /* Load Template */
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
                      'sms_phone'      => $this->input->post('sms_phone'),
					                 'comment'        => $this->input->post('comment'),
					                 'email'          => strtolower($this->input->post('email')),
					                 'imsg_enable'    => $this->input->post('imsg_enable') !== NULL ? TRUE : FALSE,
					                 'imsg_jabberid'  => $this->input->post('imsg_jabberid'),
					                 'imsg_allow_cde' => $this->input->post('imsg_allow_cde') !== NULL ? TRUE : FALSE,
					                 'password'       => $this->input->post('password')
         		        );

       if ($this->input->post('access_level') < $this->session->user_access_level)
        { $data['access_level'] = $this->input->post('access_level'); }

     		if ($this->wtd_auth_model->register($data))
		      { $this->session->set_flashdata('flash_message', 'Utilisateur '.$this->input->post('user_name').' ajouté' );
          $this->wtd_log->add("Création de l'utilisateur ".$this->input->post('user_name') );
          redirect('admin/users');
		      }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de mise à jour utilisateur' );
          redirect('admin/users');
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

    $target_user = $this->wtd_auth_model->user($id)->row();
    if (!isset($target_user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur inconnu' );
       redirect('admin/users');
     }
    if ($target_user->access_level < $this->session->user_access_level)
     { if($this->wtd_auth_model->delete($target_user->id))
 			    { $flash="Utilisateur ".$target_user->username." (".$target_user->id.") supprimé";
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression utilisateur' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Permission error' );
       $this->wtd_log->add("Tentative de suppression de l'utilisateur ".$target_user->id." (".$target_user->username.")");
     }
    redirect('admin/users');
	 }
/******************************************************************************************************************************/
	public function edit($id)
	 {	$id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		
        /* Breadcrumbs */
    $this->admin_breadcrumbs->unshift(2, lang('menu_users_edit'), 'admin/users/edit');
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
		
  		$this->data['access'] = $this->session->user_access_level;
                		/* Validate form input */
    $this->form_validation->set_rules('sms_phone', 'Téléphone', 'required');
    $this->form_validation->set_rules('email', 'E-Mail', 'required|valid_email');
    if ($this->input->post('password') OR $this->input->post('password_confirm'))
     { $this->form_validation->set_rules('password', 'Mot de passe',
                                         'required|min_length[' . $this->config->item('min_password_length') .
                                         ']|max_length[' . $this->config->item('max_password_length') .
                                         ']|matches[password_confirm]');
	      $this->form_validation->set_rules('password_confirm', 'Confirmation Mot de passe', 'required');
     }

 			if ($this->form_validation->run() == TRUE)
     {	$new_data = array( 'sms_phone'      => $this->input->post('sms_phone'),
    		                    'comment'        => $this->input->post('comment'),
		                        'email'          => strtolower($this->input->post('email')),
                          'imsg_enable'    => $this->input->post('imsg_enable') !== NULL ? TRUE : FALSE,
					                     'imsg_jabberid'  => $this->input->post('imsg_jabberid'),
					                     'imsg_allow_cde' => $this->input->post('imsg_allow_cde') !== NULL ? TRUE : FALSE,
                 			    );

       if ($this->input->post('access_level') < $this->session->user_access_level && $id!=$this->session->user_id)
        { $new_data['access_level'] = $this->input->post('access_level'); }

       if ($this->input->post('password'))	{	$new_data['password'] = $this->input->post('password');	}

       if($this->wtd_auth_model->update($id, $new_data))
 			    { $flash='Utilisateur '.$user->username.' ('.$id.') modifié';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
    						redirect('admin/users');
 			    }
       else
 			    { $flash='Erreur de mise à jour utilisateur '.$user->username;
          $this->session->set_flashdata('flash_error', $flash );
          $this->wtd_log->add( $flash );
    						redirect('admin/users');
        }
		   }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['user'] = $user = $this->wtd_auth_model->user($id)->row();
    if (!isset($user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur inconnu' );
 						redirect('admin/users');
     }
		  $this->data['user_name'] = array(	'name'  => 'username',
			                                   'id'    => 'username',
			                                   'type'  => 'text',
                                      'class' => 'form-control',
                                      'disabled' => 'TRUE',
			                                   'value' => $this->form_validation->set_value('username', $user->username)
		                                  );
		  $this->data['comment'] = array(	'name'  => 'comment',
                                    'id'    => 'comment',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('comment', $user->comment)
                                  );
    if ($id==$this->session->user_id) { $this->data['disabled'] = 'TRUE'; }
                                 else { $this->data['disabled'] = 'FALSE'; }
  		$this->data['access_level'] = array(	'name'  => 'access_level',
			                                      'id'    => 'access_level',
                                         'type'  => 'text',
                                         'class' => 'form-control',
			                                      'value' => $this->form_validation->set_value('access_level', $user->access_level)
		                                     );
		  $this->data['sms_phone'] = array( 'name'  => 'sms_phone',
			                                   'id'    => 'sms_phone',
                                      'type'  => 'tel',
                                      'pattern' => '^((\+\d{1,3}(-| )?\(?\d\)?(-| )?\d{1,5})|(\(?\d{2,6}\)?))(-| )?(\d{3,4})(-| )?(\d{4})(( x| ext)\d{1,5}){0,1}$',
                                      'class' => 'form-control',
			                                   'value' => $this->form_validation->set_value('sms_phone', $user->sms_phone)
                                  		);
		  $this->data['imsg_enable'] = array(	'name'  => 'imsg_enable',
                                        'id'    => 'imsg_enable',
                                        'checked' => $this->form_validation->set_value('imsg_enable', $user->imsg_enable)
                                      );

		  $this->data['imsg_allow_cde'] = array(	'name'  => 'imsg_allow_cde',
                                           'id'    => 'imsg_allow_cde',
                                           'checked' => $this->form_validation->set_value('imsg_allow_cde', $user->imsg_allow_cde)
                                         );

		  $this->data['imsg_jabberid'] = array(	'name'  => 'imsg_jabberid',
                                          'id'    => 'imsg_jabberid',
                                          'type'  => 'email',
                                          'class' => 'form-control',
                                          'value' => $this->form_validation->set_value('imsg_jabberid', $user->imsg_jabberid)
                                        );
		  $this->data['email'] = array(	'name'  => 'email',
                                  'id'    => 'email',
                                  'type'  => 'email',
                                  'class' => 'form-control',
                                  'value' => $this->form_validation->set_value('email', $user->email)
                                );
		  $this->data['password'] = array(	'name' => 'password',
			                                  'id'   => 'password',
                                     'class' => 'form-control',
			                                  'type' => 'password',
                                     'value' => ''
		                                 );
		  $this->data['password_confirm'] = array( 'name' => 'password_confirm',
			                                          'id'   => 'password_confirm',
                                             'class' => 'form-control',
			                                          'type' => 'password',
                                             'value' => ''
		                                         );
        /* Load Template */
		  $this->template->admin_render('admin/users/edit', $this->data);
	 }
/******************************************************************************************************************************/
	public function activate($id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $id = (int) $id;
			 $target_user = $this->wtd_auth_model->user($id)->row();
    if (!isset($target_user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur '.$id.' inconnu' );
       return($this->index());
     }
    if ($target_user->access_level < $this->session->user_access_level)
     {	$this->wtd_auth_model->activate($id);
       $this->wtd_log->add("Utilisateur ".$target_user->id." (".$target_user->username.") activé");
     }
    else { $this->wtd_log->add("Tentative d'activation de l'utilisateur ".$target_user->id." (".$target_user->username.")"); }
    redirect('admin/users');
	 }
/******************************************************************************************************************************/
	public function deactivate($id = NULL)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

  		$id = (int) $id;
			 $target_user = $this->wtd_auth_model->user($id)->row();
    if (!isset($target_user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur '.$id.' inconnu' );
       redirect('admin/users');
     }
    if ($target_user->access_level < $this->session->user_access_level)
     {	$this->wtd_auth_model->deactivate($id);
       $this->wtd_log->add("Utilisateur ".$target_user->id." (".$target_user->username.") désactivé");
     }
    else { $this->wtd_log->add("Tentative de désactivation de l'utilisateur ".$id); }
    redirect('admin/users');
 	}
/******************************************************************************************************************************/
	public function sms_activate($id)
	 {	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

  		$id = (int) $id;
			 $target_user = $this->wtd_auth_model->user($id)->row();
    if (!isset($target_user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur '.$id.' inconnu' );
       return($this->index());
     }
    if ($target_user->access_level < $this->session->user_access_level)
     {	$this->wtd_auth_model->sms_activate($id);
       $this->wtd_log->add("Utilisateur ".$target_user->id." (".$target_user->username.") SMS activé");
     }
    else { $this->wtd_log->add("Tentative d'activation SMS pour utilisateur ".$target_user->id." (".$target_user->username.") "); }
    redirect('admin/users');
	 }
/******************************************************************************************************************************/
	public function sms_deactivate($id = NULL)
 	{	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

  		$id = (int) $id;
			 $target_user = $this->wtd_auth_model->user($id)->row();
    if (!isset($target_user))
     { $this->session->set_flashdata('flash_error', 'Utilisateur '.$id.' inconnu' );
       redirect('admin/users');
     }
    if ($target_user->access_level < $this->session->user_access_level)
     {	$this->wtd_auth_model->sms_deactivate($id);
       $this->wtd_log->add("Utilisateur ".$id." SMS désactivé");
     }
    else { $this->wtd_log->add("Tentative de désactivation SMS pour utilisateur ".$target_user->id." (".$target_user->username.") "); }
    redirect('admin/users');
 	}
}
/*----------------------------------------------------------------------------------------------------------------------------*/
