<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Mnemo extends Admin_Controller {

 public function __construct()
  { parent::__construct();
    $this->load->model('Mnemo_model');
    $this->mnemo_types = array( "Bistable", "Monostable", "Temporisation", "Entrée TOR", "Sortie TOR",
                                "Entrée Analogique", "Sortie Analogique", "Visuel",
                                "Compteur horaire", "Compteur d\'impulsion", "Registre", "Horloge", "Messages" );
  }

/******************************************************************************************************************************/
 public function index($id=null)
  {	header("Content-Type: application/json; charset=UTF-8");
    switch ( $this->input->method(TRUE) )
     { case "GET":    return ($this->get($id));
       case "DELETE": return ($this->delete($id));
       case "PUT":    return ($this->update());
/*       case "POST":   return ($this->insert());*/
     }
    echo json_encode(array( "success" => "false", "error" => "Method not implemented" ));
		  exit();
  }
/******************************************************************************************************************************/
 private function get($id=NULL)
  {
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
			    exit();
		   }*/

 			$mnemo = $this->Mnemo_model->get($id);
    echo json_encode(array( "success" => "true", "Mnemo" => get_object_vars($mnemo) ));
		  exit();
  }
/******************************************************************************************************************************/
	private function delete($id=null)
 	{

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
			    exit();
		   }*/

    $mnemo = $this->Mnemo_model->get($id);
    if (!isset($mnemo))
     { echo json_encode( array( "success" => "FALSE", "error" => "Mnemo unknown" ) );
       exit();
     }

/*    if ($mnemo->access_level < $this->session->user_access_level)
     { echo json_encode( array( "success" => "FALSE", "error" => "Not authorized" ) );
       exit();
     }*/
    $this->Mnemo_model->delete($mnemo->id);
    echo json_encode(array( "success" => "true", "Mnemo" => "deleted" ));
		  exit();
	 }
/******************************************************************************************************************************/
	private function update()
	 {

   /*if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }*/

 			/*if ($this->session->user_access_level<$mnemo->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/mnemo/index/'.$mnemo->dls_id);
     }*/

    $input = json_decode(file_get_contents('php://input'));
    if ($input==NULL) { echo json_encode( array( "success" => "false", "error" => "Parsing Error !" ) ); exit(); }

    $data = array( 'libelle'   => $input->libelle,
		                 'ev_host'   => $input->ev_host,
                   'ev_thread' => strtoupper($input->ev_thread),
				               'ev_text'   => $input->ev_text,
                 );
    if($this->Mnemo_model->update($input->id, $data))
     { echo json_encode( array( "success" => "true", "Mnemo" => "Updated !" ) );
       if($mnemo->ev_thread=="VOICE"  || $data['ev_thread']=="VOICE")  { $this->wtd_webservice->send('/reload/voice'); }
       if($mnemo->ev_thread=="MODBUS" || $data['ev_thread']=="MODBUS") { $this->wtd_webservice->send('/reload/modbus'); }
	    }
    else { echo json_encode( array( "success" => "false", "error" => "Update error !" ) ); }
    exit();
	 }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
