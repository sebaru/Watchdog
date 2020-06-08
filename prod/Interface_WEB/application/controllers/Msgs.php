<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Msgs extends Public_Controller {

   public function __construct()
    { parent::__construct();
      $this->data["load_js"] = "message.js";
      $this->load->model('Msg_model');
    }
/******************************************************************************************************************************/
 public function index ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->data["body_ready"] = "Charger_ws_msgs()";
    $this->template->public_render('public/msgs', $this->data);
 	}
/******************************************************************************************************************************/
 public function activite ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->data["msgs"] = $this->Msg_model->get_alive("activite");
    $this->data["body_ready"] = "Charger_ws_msgs()";
    $this->template->public_render('public/msgs', $this->data);
 	}
/******************************************************************************************************************************/
 public function secubien ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->data["msgs"] = $this->Msg_model->get_alive("secubien");
    $this->data["body_ready"] = "Charger_ws_msgs()";
    $this->template->public_render('public/msgs', $this->data);
 	}
/******************************************************************************************************************************/
 public function secupers ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->data["msgs"] = $this->Msg_model->get_alive("secupers");
    $this->data["body_ready"] = "Charger_ws_msgs()";
    $this->template->public_render('public/msgs', $this->data);
 	}
/******************************************************************************************************************************/
 public function acquit ($tech_id=NULL,$acronyme=NULL)
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    if( $tech_id && $acronyme )
     { $this->wtd_webservice->post("/histo/ack", array ( "tech_id" => $tech_id, "acronyme" => $acronyme ) );
     }
 	}
/******************************************************************************************************************************/
 public function histos ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $this->data["msgs"] = $this->Msg_model->get_histo($this->input->post("search"));
    $this->template->public_render('public/histos', $this->data);
 	}
}
