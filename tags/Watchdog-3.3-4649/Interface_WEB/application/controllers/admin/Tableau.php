<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Tableau extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Tableau_model');

		  /* Title Dls :: Common */
    $this->admin_page_title->push('Tableaux');
    $this->data['pagetitle'] = "<h1>Gestions des tableaux</h1>";
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Tableaux', 'admin/tableau');
  }

/******************************************************************************************************************************/
 function index()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
    $this->data['tableaux'] = $this->Tableau_model->get_all();                                                 /* Get all ups */
			 /* Load Template */
			 $this->template->admin_render('admin/tableau/index', $this->data);
		}
/******************************************************************************************************************************/
 function create()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

   	$data = array( 'titre'          => "Nouveau tableau",
       		        );
	   $tableau = $this->Tableau_model->add($data);
    $flash = 'Le tableau <strong>'.$tableau->id.'</strong> a été ajouté.';
	   $this->session->set_flashdata('flash_message', $flash);
    $this->wtd_log->add($flash);
    redirect('admin/tableau/index');
  }
/******************************************************************************************************************************/
	public function delete($id)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target = $this->Tableau_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Tableau '.$id.' inconnu' );
       redirect('admin/tableau/index');
     }
    if ($target->access_level < $this->session->user_access_level)
     { if($this->Tableau_model->delete($target->id))
 			    { $flash = 'Tableau <strong>'.$target->titre.'</strong> ('.$target->id.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression du Tableau' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression du tableau ".
                           '<strong>'.$target->titre.'</strong> ('.$target->id.')');
     }
    redirect('admin/tableau/index');
	 }
/******************************************************************************************************************************/
 function edit($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/tableau/index');
     }

			 // check if the tableau exists before trying to edit it
			 $this->data['tableau'] = $tableau = $this->Tableau_model->get($id);
 			if(!isset($tableau))
     { $this->session->set_flashdata('flash_error', "Ce tableau n'existe pas");
       redirect('admin/tableau/index');
     }

 			if ($this->session->user_access_level<$tableau->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/tableau/index');
     }

    $this->admin_breadcrumbs->unshift(2, 'Editer un tableau', 'admin/tableau/edit/'.$id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

				$this->form_validation->set_rules('titre','titre','required');

   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'titre'        => $this->input->post('titre'),
                        'access_level' => $this->input->post('access_level'),
                 					);

	  				$this->Tableau_model->update($id,$params);
       $flash = 'Le tableau <strong>'.$this->input->post('titre').'</strong>/ a été modifié.';
  					$this->session->set_flashdata('flash_message', $flash);
       $this->wtd_log->add($flash);
       redirect('admin/tableau/index');
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

    $this->data['courbes'] = $this->Tableau_model->get_courbes($tableau->id);                                  /* Get all ups */
    $this->data["access_level"] = $tableau->access_level;
		  $this->data['titre'] = array(	'name'  => 'titre',
	                                 'id'    => 'titre',
	                                 'type'  => 'text',
                                  'class' => 'form-control',
	                                 'value' => $this->form_validation->set_value('titre', $tableau->titre)
	                               );
        /* Load Template */
		  $this->template->admin_render('admin/tableau/edit', $this->data);
 	}
/******************************************************************************************************************************/
 function add_courbe($tableau_id)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

   	$data = array( 'tableau_id'       => $tableau_id,
                   'tech_id'          => "New_tech_id",
                   'acronyme'         => "New_acronyme",
                   'color'            => "#00f",
       		        );
	   $this->Tableau_model->add_courbe($data);
    $flash = 'Courbe ajoutée sur le tableau <strong>'.$tableau_id.'</strong>';
	   $this->session->set_flashdata('flash_message', $flash);
    $this->wtd_log->add($flash);
    redirect('admin/tableau/edit/'.$tableau_id);
  }
/******************************************************************************************************************************/
 function set_courbe($tableau_id, $courbe_id)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

   	$data = array( 'tech_id'          => $this->input->post("tech_id"),
                   'acronyme'         => $this->input->post("acronyme"),
                   'color'            => $this->input->post("color"),
       		        );
	   $this->Tableau_model->set_courbe($courbe_id,$data);
    $flash = 'Courbe modifiée sur le tableau <strong>'.$tableau_id.'</strong>';
	   $this->session->set_flashdata('flash_message', $flash);
    $this->wtd_log->add($flash);
    redirect('admin/tableau/edit/'.$tableau_id);
  }
/******************************************************************************************************************************/
 function del_courbe($tableau_id, $courbe_id)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

	   $this->Tableau_model->del_courbe($courbe_id);
    $flash = 'Courbe '.$courbe_id.' supprimée sur le tableau <strong>'.$tableau_id.'</strong>';
	   $this->session->set_flashdata('flash_message', $flash);
    $this->wtd_log->add($flash);
    redirect('admin/tableau/edit/'.$tableau_id);
  }
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
