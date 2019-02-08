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
  { header("Content-Type: application/json; charset=UTF-8");
    switch ( $this->input->method(TRUE) )
     { case "GET":    return ($this->get($id));
       case "DELETE": return ($this->delete($id));
       case "PUT":    return ($this->update());
/*       case "POST":   return ($this->insert());*/
     }
    echo json_encode(array( "success" => "FALSE", "error" => "Method not implemented" ));
    exit();
  }
/******************************************************************************************************************************/
 public function list($id=NULL)
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
    if (isset($mnemos))
     { foreach($mnemos as $mnemo)
        { $data[] = get_object_vars( $mnemo ); }
     }
    echo json_encode(array( "success" => "true", "Mnemos" => $data));
    exit();
  }
/******************************************************************************************************************************/
 public function voices()
  { header("Content-Type: application/json; charset=UTF-8");

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $data = array();
    $mnemos = $this->Mnemo_model->get_voice();
    if (isset($mnemos))
     { foreach($mnemos as $mnemo)
        { $data[] = get_object_vars( $mnemo ); }
     }
    echo json_encode(array( "success" => "true", "Voices" => $data));
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
/******************************************************************************************************************************/
 private function get($id=NULL)
  {
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $mnemo = $this->Mnemo_model->get($id);
    if (!isset($mnemo))
     { echo json_encode( array( "success" => "FALSE", "error" => "Mnemo unknown" ) ); }
    else
     { echo json_encode( array( "success" => "TRUE", "Mnemo" => get_object_vars($mnemo) )); }
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
    echo json_encode(array( "success" => "TRUE", "Mnemo" => "deleted" ));
    $this->wtd_log->add('Le mnemo '.$mnemo->id.' a été supprimé.');
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
    if (!isset($input)) { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }

    $mnemo = $this->Mnemo_model->get($input->id);
    if (!isset($mnemo)) { echo json_encode( array( "success" => "FALSE", "error" => "Mnemo unknown !" ) ); exit(); }

    $data = array();
    if (isset($input->libelle))   $data['libelle']   = $input->libelle;
    if (isset($input->ev_host))   $data['ev_host']   = $input->ev_host;
    if (isset($input->ev_thread)) $data['ev_thread'] = strtoupper($input->ev_thread);
                             else $data['ev_thread'] = $mnemo->ev_thread;
    if (isset($input->ev_text))   $data['ev_text']   = $input->ev_text;

    if($this->Mnemo_model->update($mnemo->id, $data))
     { echo json_encode( array( "success" => "TRUE", "Mnemo" => "Updated !" ) );
       if($mnemo->ev_thread=="VOICE"  || $data['ev_thread']=="VOICE")  { $this->wtd_webservice->send('/reload/voice'); }
       if($mnemo->ev_thread=="MODBUS" || $data['ev_thread']=="MODBUS") { $this->wtd_webservice->send('/reload/modbus'); }
       $flash = 'Le mnémo '.$mnemo->tech_id.':'.$mnemo->acronyme.' a été updaté.';
       $this->wtd_log->add($flash);
     }
    else { echo json_encode( array( "success" => "FALSE", "error" => "Update error !" ) ); }
    exit();
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
