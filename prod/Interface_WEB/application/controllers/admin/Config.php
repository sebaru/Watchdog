<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Config extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Config_model');

		  /* Title Dls :: Common */
    $this->admin_page_title->push('Configuration');
    $this->data['pagetitle'] = "<h1>Configuration des threads</h1>";
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Configuration', 'admin/config');
  }

/******************************************************************************************************************************/
 function index($host=NULL,$thread=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
    $this->data['configs'] = $this->Config_model->get_list($host,$thread);                         /* Get all running process */
    $this->data['host']    = $host;
    $this->data['thread']  = $thread;
    $this->template->admin_render('admin/config/index', $this->data);                                        /* Load Template */
		}
/******************************************************************************************************************************/
 function set($host=NULL,$thread=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
		  $param  = $this->input->post("parametre");
    $valeur = $this->input->post("valeur");
    $this->Config_model->set($host,$thread,$param,$valeur);                                        /* Get all running process */
    $this->wtd_webservice->send('/process/reload/'.$thread);
    $this->session->set_flashdata('flash_message', 'Paramètre '.$param.' ajouté' );
    redirect('admin/config/index/'.$host.'/'.$thread);                                                       /* Load Template */
		}
/******************************************************************************************************************************/
 public function delete($host=NULL,$thread=NULL,$param=NULL)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $this->Config_model->del($host,$thread,$param,$valeur);                                        /* Get all running process */
    $this->session->set_flashdata('flash_message', 'Paramètre '.$param.' supprimé' );
    redirect('admin/config/index/'.$host.'/'.$thread);                                                       /* Load Template */
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
