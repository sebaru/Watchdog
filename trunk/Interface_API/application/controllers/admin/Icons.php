<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Icons extends Admin_Controller {

/******************************************************************************************************************************/
	function __construct()
	 {	parent::__construct();
    $this->load->model('Icons_model');
    $this->admin_page_title->push('Gestion des Icones');
    $this->admin_breadcrumbs->unshift(1, 'Classes et Icones', 'admin/icons');
    $this->data['pagetitle'] = "<h1>Classes et Icones</h1>";
    /* Breadcrumbs :: Common */
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $this->data['classes'] = $this->Icons_model->class_list();                                               /* Get all class */

    $this->template->admin_render('admin/icons/class_list', $this->data);                                    /* Load Template */
 	}
/******************************************************************************************************************************/
	function class_create()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
 			$this->form_validation->set_rules('libelle','Libellé','required');
			
 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/icons/index','refresh');
     }

    $this->admin_breadcrumbs->unshift(2, 'Creer une classe', 'admin/icons/class_create');
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

 			if($this->form_validation->run())     
			  { $params = array( 'libelle' => $this->input->post('libelle'),
				                  );
				
				   $classe_id = $this->Icons_model->class_create($params);
       $flash = 'La classe <strong>'.$this->input->post('libelle').'</strong> ('.$classe_id.') a été ajoutée.';
	      $this->session->set_flashdata('flash_message', $flash);
       $this->wtd_log->add($flash);
       redirect('admin/icons/index');
     }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['libelle'] = array(	'name'  => 'libelle',
	                                   'id'    => 'libelle',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
	                                   'value' => $this->form_validation->set_value('libelle', $this->input->post('libelle'))
                                  );

    $this->template->admin_render('admin/icons/class_create', $this->data);                                  /* Load Template */
 	}
/******************************************************************************************************************************/
	function class_edit($class_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/icons/index');
     }
		
    $this->admin_breadcrumbs->unshift(2, 'Editer une classe', 'admin/icons/class_edit/'.$class_id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

			 // check if the dls exists before trying to edit it
			 $this->data['class'] = $class = $this->Icons_model->class_get($class_id);
 			if(!isset($class))
     { $this->session->set_flashdata('flash_error', "Cette classe n'existe pas");
       redirect('admin/icons/index');
     }

 			$this->form_validation->set_rules('libelle','Libellé','required');
			
 			if($this->form_validation->run())     
			  { $params = array( 'libelle' => $this->input->post('libelle'),
				                  );
				
				   if($this->Icons_model->class_update($class->id, $params)==TRUE)
        { $flash = 'La classe <strong>'.$this->input->post('libelle').'</strong> ('.$class->id.') a été modifiée.';
				      $this->session->set_flashdata('flash_message', $flash);
        }
       else
        { $flash = 'Erreur lors de la modification de la classe <strong>'.$class->libelle.'</strong> ('.$class->id.').';
				      $this->session->set_flashdata('flash_error', $flash);
        }
       $this->wtd_log->add($flash);
       redirect('admin/icons/index');
     }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['libelle'] = array(	'name'  => 'libelle',
	                                   'id'    => 'libelle',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
	                                   'value' => $this->form_validation->set_value('libelle', $class->libelle)
                                  );

    $this->template->admin_render('admin/icons/class_edit', $this->data);                                    /* Load Template */
 	}
/******************************************************************************************************************************/
	function icon_edit($icon_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/icons/index');
     }
		
    $this->admin_breadcrumbs->unshift(3, 'Editer un icone', 'admin/icons/icon_edit/'.$icon_id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

			 // check if the dls exists before trying to edit it
			 $this->data['icon'] = $icon = $this->Icons_model->icon_get($icon_id);
 			if(!isset($icon))
     { $this->session->set_flashdata('flash_error', "Cet icone n'existe pas");
       redirect('admin/icons/index');
     }

 			$this->form_validation->set_rules('libelle','Libellé','required');
			
 			if($this->form_validation->run())     
			  { $params = array( 'libelle' => $this->input->post('libelle'),
				                  );

				   if($this->Icons_model->icon_update($icon->id, $params)==TRUE)
        { $flash = 'Cet icone <strong>'.$this->input->post('libelle').'</strong> ('.$icon->id.') a été modifié.';
				      $this->session->set_flashdata('flash_message', $flash);
        }
       else
        { $flash = 'Erreur lors de la modification de la classe <strong>'.$class->libelle.'</strong> ('.$class->id.').';
				      $this->session->set_flashdata('flash_error', $flash);
        }
       $this->wtd_log->add($flash);
       redirect('admin/icons/icon_list/'.$icon->id_classe);
     }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['libelle'] = array(	'name'  => 'libelle',
	                                   'id'    => 'libelle',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
	                                   'value' => $this->form_validation->set_value('libelle', $icon->libelle)
                                  );

    $this->data['nbr_matrice'] = array(	'name'  => 'nbr_matrice',
	                                       'id'    => 'nbr_matrice',
	                                       'type'  => 'text',
                                        'class' => 'form-control',
                                        'disabled' => 'TRUE',
	                                       'value' => $this->form_validation->set_value('nbr_matrice', $icon->nbr_matrice)
                                  );

    $this->data['new_file'] = array(	'name'  => 'new_file',
	                                    'id'    => 'new_file',
                                     'class' => 'form-control',
                                  );

    $this->template->admin_render('admin/icons/icons_edit', $this->data);                                    /* Load Template */
 	}
/******************************************************************************************************************************/
	function icon_upload($icon_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/icons/index');
     }
		
			 // check if the dls exists before trying to edit it
			 $this->data['icon'] = $icon = $this->Icons_model->icon_get($icon_id);
 			if(!isset($icon))
     { $this->session->set_flashdata('flash_error', "Cet icone n'existe pas");
       redirect('admin/icons/index');
     }
		
			 /*$params = array( 'libelle' => $this->input->post('libelle'),
				               );*/

    $config['upload_path']          = 'assets/gif/';
    $config['allowed_types']        = 'gif|svg|01|02|03|04|05|06|07|08|09';
    if ($icon->nbr_matrice==0)
     { $config['file_name']            = $icon->id.'.gif'; }
    else
     { $config['file_name']            = $icon->id.'.gif.'.sprintf("%02d", $icon->nbr_matrice); }
    $config['max_size']   = 1024;
    $config['max_width']  = 256;
    $config['max_height'] = 256;
    $config['overwrite']  = TRUE;
    $config['mod_mime_fix']  = FALSE;
    $this->load->library('upload', $config);
				
    if ( ! $this->upload->do_upload('new_file') )
     { $this->session->set_flashdata('flash_error', $this->upload->display_errors() );
       redirect('admin/icons/icon_list/'.$icon->id_classe);
     }
    $params['nbr_matrice'] = $icon->nbr_matrice+1;

	   $this->Icons_model->icon_update($icon->id, $params);
    $flash = 'Cet icone <strong>'.$icon->libelle.'</strong> ('.$icon->id.') a été modifié.';
	   $this->session->set_flashdata('flash_message', $flash);
    $this->wtd_log->add($flash);
    redirect('admin/icons/icon_list/'.$icon->id_classe);
 	}
/******************************************************************************************************************************/
	public function class_delete($id)
 	{ $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_class = $this->Icons_model->class_get($id);
    if (!isset($target_class))
     { $this->session->set_flashdata('flash_error', 'Classe '.$id.' inconnue' );
       redirect('admin/icons/index');
     }
    if ($this->session->user_access_level >= 6)
     { if($this->Icons_model->class_delete($target_class->id))
 			    { $flash = 'Classe ('.$target_class->id.', '.$target_class->libelle.') supprimée';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add( $flash );
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression Classe' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression classe ".$target_class->id.', '.$target_class->libelle);
     }
    redirect('admin/icons/index');
	 }
/******************************************************************************************************************************/
	function icon_list($id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $this->data['icons'] = $icon = $this->Icons_model->icons_list($id);                                      /* Get all icons */
    $this->data['class_id'] = $id;                                                                           /* Get all icons */

    $this->admin_breadcrumbs->unshift(3, 'Liste des icones', 'admin/icons/icon_list'.$id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

    $this->template->admin_render('admin/icons/icons_list', $this->data);                                    /* Load Template */
 	}
/******************************************************************************************************************************/
	public function icon_delete($id_classe,$id)
 	{ $id = (int) $id;
   	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_icon = $this->Icons_model->icon_get($id);
    if (!isset($target_icon))
     { $this->session->set_flashdata('flash_error', 'Icone '.$id.' inconnu' );
       redirect('admin/icons/icon_list/'.$id_classe);
     }
    if ($this->session->user_access_level >= 6)
     { if($this->Icons_model->icon_delete($target_icon->id))
 			    { $flash = 'Icone ('.$target_icon->id.', '.$target_icon->libelle.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add( $flash );
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression Icone' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression icone ".$target_icon->id.', '.$target_icon->libelle);
     }
    redirect('admin/icons/icon_list/'.$id_classe);
	 }
/******************************************************************************************************************************/
	public function icon_create($id_classe)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_class = $this->Icons_model->class_get($id_classe);
    if (!isset($target_class))
     { $this->session->set_flashdata('flash_error', 'Classe '.$id_classe.' inconnue' );
       redirect('admin/icons/index');
     }

    if ($this->session->user_access_level >= 6)
     { $params['id_classe'] = $id_classe;
       $params['libelle']   = "Nouvel Icone";
       $icon_id = $this->Icons_model->icon_add($params);
       if($icon_id>0)
 			    { $flash = 'Icone '.$icon_id.' a été ajouté dans la classe '.$target_class->libelle.' ('.$target_class->id.')';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add( $flash );
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur d\'ajout d\'icone' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative d'ajout d'icone");
     }
    redirect('admin/icons/icon_edit/'.$icon_id);
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
