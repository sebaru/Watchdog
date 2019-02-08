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
     { $data[] = get_object_vars( $mnemo ); }
    echo json_encode(array( "success" => "true", "Mnemos" => $data));
    exit();
  }
/******************************************************************************************************************************/
 public function count()
  { header("Content-Type: application/json; charset=UTF-8");

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
			    exit();
		   }*/

    $count = $this->Mnemo_model->get_count();
    echo json_encode(array( "success" => "true", "count" => $count));
    exit();
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
