<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Syn extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Syn_model');
    $this->load->model('Dls_model');
  } 
/******************************************************************************************************************************/
 public function index($syn_id=NULL)
  { header("Content-Type: application/json; charset=UTF-8");
    switch ( $this->input->method(TRUE) )
     { case "GET":    return ($this->get($syn_id));
       case "DELETE": return ($this->delete($syn_id));
       case "PUT":    return ($this->update());
       case "POST":   return ($this->create());
     }
    echo json_encode(array( "success" => "FALSE", "error" => "Method not implemented" ));
    exit();
  }
/******************************************************************************************************************************/
 public function list ()
  { header("Content-Type: application/json; charset=UTF-8");

    $data = array();
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode(array( "draw"=>$draw, "recordsTotal" => 0, "recordsFiltered" => 0, "data" => $data ));
       exit();
     }*/

  		$syns = $this->Syn_model->get_all();
		  if (isset($syns))
     { foreach($syns as $syn)
        { $data[] = get_object_vars( $syn ); }
     }
    echo json_encode(array( "success" => "true", "Syns" => $data));
    exit();
  }
/******************************************************************************************************************************/
 public function motifs ($id)
  { header("Content-Type: application/json; charset=UTF-8");

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode(array( "draw"=>$draw, "recordsTotal" => 0, "recordsFiltered" => 0, "data" => $data ));
       exit();
     }*/

    $data_motifs = array();
  		$motifs = $this->Syn_model->motifs($id);
		  if (isset($motifs))
     { foreach($motifs as $motif)
        { $data_motifs[] = get_object_vars( $motif ); }
     }

    $data_comments = array();
  		$comments = $this->Syn_model->comments($id);
		  if (isset($comments))
     { foreach($comments as $comment)
        { $data_comments[] = get_object_vars( $comment ); }
     }

    $data_passerelles = array();
  		$passerelles = $this->Syn_model->passerelles($id);
		  if (isset($passerelles))
     { foreach($passerelles as $passerelle)
        { $data_passerelles[] = get_object_vars( $passerelle ); }
     }

    echo json_encode(array( "success" => "true",
                            "Motifs" => $data_motifs,
                            "Comments" => $data_comments,
                            "Passerelles" => $data_passerelles )
                    );
    exit();
  }
/******************************************************************************************************************************/
 private function get($id=NULL)
  {
/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $syn = $this->Syn_model->get($id);
    if (!isset($syn))
     { echo json_encode( array( "success" => "FALSE", "error" => "SYN unknown" ) ); }
    else
     { echo json_encode( array( "success" => "TRUE", "SYN" => get_object_vars($syn) )); }
    exit();
  }
/******************************************************************************************************************************/
 private function delete($id=null)
  {

/*    if ( ! $this->wtd_auth->logged_in() )
     { echo json_encode( array( "success" => "FALSE", "error" => "need to be authenticated" ) );
       exit();
     }*/

    $syn = $this->Syn_model->get($id);
    if (!isset($syn))
     { echo json_encode( array( "success" => "FALSE", "error" => "Syn unknown" ) );
       exit();
     }

/*    if ($mnemo->access_level < $this->session->user_access_level)
     { echo json_encode( array( "success" => "FALSE", "error" => "Not authorized" ) );
       exit();
     }*/
    $this->Syn_model->delete($syn->id);
    $this->wtd_webservice->send('/reload/dls');
    echo json_encode(array( "success" => "TRUE", "SYN" => "deleted" ));
    $this->wtd_log->add('Le synoptique '.$syn->id.' a été supprimé.');
    exit();
  }
/******************************************************************************************************************************/  
 private function create($syn_id=NULL)
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

    $data = array();

    if (isset($input->page))         $data['page']         = strtoupper($input->page);
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->libelle))      $data['libelle']      = $input->libelle;
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->access_level)) $data['access_level'] = $input->access_level;
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }
    if (isset($input->parent_id))    $data['parent_id']    = $input->parent_id;
    else { echo json_encode( array( "success" => "FALSE", "error" => "Parsing Error !" ) ); exit(); }

    $syn_id = $this->Syn_model->add($data);
    echo json_encode( array( "success" => "TRUE", "SYN" => "Added !" ) );
    $flash = 'Le dls '.$input->page.' ('.$syn_id.') a été ajouté.';
    $this->wtd_log->add($flash);
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

    $syn = $this->Syn_model->get($input->id);
    if (!isset($syn)) { echo json_encode( array( "success" => "FALSE", "error" => "SYN unknown !" ) ); exit(); }

    $data = array();
    if (isset($input->page))         $data['page']         = strtoupper($input->page);
    if (isset($input->libelle))      $data['libelle']      = $input->libelle;
    if (isset($input->access_level)) $data['access_level'] = $input->access_level;

    if($this->Syn_model->update($syn->id, $data))
     { echo json_encode( array( "success" => "TRUE", "SYN" => "Updated !" ) );
       $flash = 'Le synoptique '.$syn->page.' ('.$syn->id.') a été updaté.';
       $this->wtd_log->add($flash);
     }
    else { echo json_encode( array( "success" => "FALSE", "error" => "Update error !" ) ); }
    exit();
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/
