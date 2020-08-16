<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Tableau extends Public_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Tableau_model');

		  /* Title Dls :: Common */
    $this->admin_page_title->push('Tableaux');
    $this->data['pagetitle'] = "<h1>Gestions des tableaux</h1>";
  }

/******************************************************************************************************************************/
 function index()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $this->data['tableaux'] = $this->Tableau_model->get_all();                                                 /* Get all ups */
			 /* Load Template */
			 $this->template->public_render('public/tableau_index', $this->data);
		}
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
