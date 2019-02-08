<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemos extends Admin_Controller {

 public function __construct()
  { parent::__construct();
    $this->load->model('Mnemo_model');
    $this->mnemo_types = array( "Bistable", "Monostable", "Temporisation", "Entrée TOR", "Sortie TOR",
                                "Entrée Analogique", "Sortie Analogique", "Visuel",
                                "Compteur horaire", "Compteur d\'impulsion", "Registre", "Horloge", "Messages" );
  }
/******************************************************************************************************************************/
 public function index($id=NULL)
  { header("Content-Type: application/json; charset=UTF-8");

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
			    exit();
		   }*/

    if (!isset($id))
     { echo json_encode( array( "success" => "FALSE", "error" => "need DLS id" ) );
       exit();
     }

    $data = array();
    $mnemos = $this->Mnemo_model->get_all($id);
    foreach($mnemos as $mnemo)
     { $data[] = array( "id" => $mnemo->id,
                        "tech_id" => $mnemo->tech_id,
                        "acronyme" => $mnemo->acronyme,
                        "type" => $this->mnemo_types[$mnemo->type],
                        "libelle" => $mnemo->libelle,
                        "ev_host" => $mnemo->ev_host,
                        "ev_thread" => $mnemo->ev_thread,
                        "ev_text" => $mnemo->ev_text,
                      );
     }
    echo json_encode(array( "success" => "true", "Mnemos" => $data));
    exit();
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
