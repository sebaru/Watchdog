<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Horloges extends Public_Controller {

/************************************** Chargement des classes et icones pour le client lourd *********************************/
	function __construct()
	 {	parent::__construct();
    $this->load->model('Mnemo_model');
    $this->load->model('Ticks_model');
  }

/******************************************************************************************************************************/
	public function index()
	 { redirect('horloges/list');
 	}
/******************************************************************************************************************************/
	public function list($page_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['horloges'] = $this->Mnemo_model->get_HORLOGE_page($page_id);                                 /* Get horloges */
    $this->template->public_render('public/horloges_index', $this->data);                                    /* Load Template */
 	}
/******************************************************************************************************************************/
	public function show($tech_id=NULL, $acronyme=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
    $this->data['horloge'] = $horloge = $this->Mnemo_model->get_HORLOGE($tech_id,$acronyme);            /* Get horloges */
    $this->data['ticks'] = $this->Ticks_model->get($horloge->id);                                       /* Get horloges */
    $this->template->public_render('public/horloges_ticks', $this->data);                                     /* Load Template */
 	}
/******************************************************************************************************************************/
	public function create_tick($tech_id, $acronyme, $horloge_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
   	$data = array( 'horloge_id'     => $horloge_id,
                   'heure'          => "12",
                   'minute'         => "00",
       		        );

  		if ($this->Ticks_model->add($data))
     { $flash = "New Tick added";
       $this->wtd_log->add($flash);
       $this->session->set_flashdata('flash_message', $flash );
     }
    else
	    { $this->session->set_flashdata('flash_error', "Erreur lors de l'ajout d'un tick Horloge" );
		   }
            /* Load Template */
    redirect('horloges/show/'.$tech_id.'/'.$acronyme);
 	}
/******************************************************************************************************************************/
 public function delete($tech_id, $acronyme, $tick_id=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $this->Ticks_model->delete($tick_id);
    $flash = "Tick ".$tick_id." supprimé";
    $this->session->set_flashdata('flash_message', $flash );
    $this->wtd_log->add( $flash );
    redirect('horloges/show/'.$tech_id.'/'.$acronyme);
  }
/******************************************************************************************************************************/
 public function set($tech_id, $acronyme, $tick_id=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    $data = array( 'heure'      => $this->input->post('heure'),
                   'minute'     => $this->input->post('minute'),
                 );
    if ($this->Ticks_model->set($tick_id, $data))
     { $flash = "Tick ".$tick_id." modifié";
       $this->session->set_flashdata('flash_message', $flash );
       $this->wtd_log->add( $flash );
     }
    else
     { $flash = "Tick ".$tick_id." en erreur";
       $this->session->set_flashdata('flash_message', $flash );
       $this->wtd_log->add( $flash );
     }
    redirect('horloges/show/'.$tech_id.'/'.$acronyme);
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
