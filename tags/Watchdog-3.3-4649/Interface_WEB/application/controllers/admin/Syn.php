<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Syn extends Admin_Controller{
    function __construct()
     { parent::__construct();
       $this->load->model('Syn_model');
       $this->data["load_js"] = "atelier.js";
		/* Title Syn :: Common */
       $this->admin_page_title->push('Synoptiques');
       $this->data['pagetitle'] = "<h1>Synoptiques</h1>";
        /* Breadcrumbs :: Common */
       $this->admin_breadcrumbs->unshift(1, 'Synoptiques', 'admin/syn');
     }

/******************************************************************************************************************************/
    function index()
     { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    			$this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
			    /* Load Template */
			    $this->template->admin_render('admin/syn/index', $this->data);
     }

/******************************************************************************************************************************/
 function get()
  {	header("Content-Type: application/json; charset=UTF-8");
		  $draw   = intval($this->input->get("draw"));
    $start  = intval($this->input->get("start"));
		  $length = intval($this->input->get("length"));

		  $data = array();

		  if ( ! $this->wtd_auth->logged_in() )
     {	echo json_encode(array(	"draw"=>$draw,	"recordsTotal" => 0, "recordsFiltered" => 0,	"data" => $data ));
			    exit();
		   }

  		$syns = $this->Syn_model->get_all($start, $length);
		  foreach($syns->result() as $syn)
     {	$caret = '<div class="dropdown">'.
                '<button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">'.
                '<span class="caret"></span></button>'.
                '<ul class="dropdown-menu">'.
                '<li><a href='.site_url('admin/syn/edit/'.$syn->id).'>'.
                    '<i class="fa fa-pencil" style="color:green"></i>'.
                    '<i class="fa fa-image" style="color:blue"></i>Configurer</a></li>'.
                '<li><a href='.site_url('admin/syn/atelier/'.$syn->id).'>'.
                    '<i class="fa fa-pencil" style="color:green"></i>'.
                    '<i class="fa fa-image" style="color:blue"></i>Atelier</a></li>'.
                '<li><a href='.site_url('admin/syn/create/'.$syn->id).'>'.
                    '<i class="fa fa-plus" style="color:green"></i>'.
                    '<i class="fa fa-image" style="color:blue"></i>Creer un fils</a></li>'.
                '<li><a href='.site_url('admin/dls/create/'.$syn->id).'>'.
                    '<i class="fa fa-plus" style="color:green"></i>'.
                    '<i class="fa fa-cog" style="color:blue"></i>Creer un DLS</a></li>'.
                '<li class="divider"></li>'.
                '<li><a href='.site_url('admin/syn/delete/'.$syn->id).'>'.
                    '<i class="fa fa-times" style="color:red"></i>Supprimer ce synoptique</a></li>'.
                '</ul></div>';
       $data[] = array(	$caret, $syn->id,	$syn->access_level, $syn->ppage, $syn->page,
                      	'<a href='.site_url('admin/syn/edit/'.$syn->id).' data-toggle="tooltip" style="cursor:pointer" title="Configurer">'.$syn->libelle.'</a>',
                      );
		   }

		  $total_syns = $this->Syn_model->get_total();

		  $output = array( "draw" => $draw,	"recordsTotal" => $total_syns,	"recordsFiltered" => $total_syns,	"data" => $data	);
		  echo json_encode($output);
		  exit();
	 }

/******************************************************************************************************************************/
 function create($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       return($this->index());
     }

			// check if the syn exists before trying to edit it
 			$syn = $this->Syn_model->get($id);
 			if(!isset($syn))
     { $this->session->set_flashdata('flash_error', "Ce synoptique n'existe pas");
       return($this->index());
     }

	 		$params = array(	'access_level' => 0,
			                  'libelle' => "Nouveau synoptique",
			                  'page' => "NEW",
                     'parent_id' => $id,
		                 );

				$new_syn_id = $this->Syn_model->create($params);
    $message = 'Le synoptique '.$new_syn_id.' a été ajouté.';
				$this->session->set_flashdata('flash_message',$message);
    $this->wtd_log->add($message);
				redirect('admin/syn', 'refresh');
  }

/******************************************************************************************************************************/
 function edit($id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       return($this->index());
     }

			// check if the syn exists before trying to edit it
 			$syn = $this->data['syn'] = $this->Syn_model->get($id);
 			if(!isset($syn))
     { $this->session->set_flashdata('flash_error', "Ce synoptique n'existe pas");
       return($this->index());
     }

				$this->admin_breadcrumbs->unshift(2, 'Editer un synoptique', 'admin/syn/edit');
				$this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
				$this->data['syns'] = $this->Syn_model->get_all(0,0);

				$this->form_validation->set_rules('access_level','Access Level','required');
				$this->form_validation->set_rules('libelle','Libellé','required');
				$this->form_validation->set_rules('page','Page','required');

				if($this->form_validation->run())
		 		{	$params = array(	'access_level' => $this->input->post('access_level'),
						                  'libelle' => $this->input->post('libelle'),
						                  'page' => $this->input->post('page'),
					                 );

  					$this->Syn_model->update($id,$params);
					//redirect('admin/syn');
       $this->session->set_flashdata('flash_message','Le synoptique '.$params['page'].' a été modifié.');
       $this->wtd_log->add('Le synoptique '.$params['page'].' a été modifié.');
 						redirect('admin/syn', 'refresh');
				 }
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

		  $this->data['ppage'] = array(	'name'  => 'ppage',
			                               'id'      => 'ppage',
			                               'type'    => 'text',
                                  'class'   => 'form-control',
                                  'disabled' => 'TRUE',
			                               'value'   => $this->form_validation->set_value('ppage', $syn->ppage)
		                              );
		  $this->data['access_level'] = array(	'name'  => 'access_level',
                                      'id'    => 'access_level',
                                      'type'  => 'number',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('access_level', $syn->access_level)
                                    );
		  $this->data['page'] = array(	'name'  => 'page',
                                 'id'    => 'page',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('page', $syn->page)
                               );

		  $this->data['libelle'] = array(	'name'  => 'libelle',
                                    'id'    => 'libelle',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('libelle', $syn->libelle)
                                  );
        /* Load Template */
		  $this->template->admin_render('admin/syn/edit', $this->data);
	}

/******************************************************************************************************************************/
	function delete($id)
  {	if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
 			if ($id==1)
     { $this->session->set_flashdata('flash_error', 'Suppression du synoptique racine interdite !');
       redirect('admin/syn/index');
     }


    $target = $this->Syn_model->get($id);
    if (!isset($target))
     { $this->session->set_flashdata('flash_error', 'Synoptique '.$id.' inconnu' );
       redirect('admin/syn/index');
     }
    if ($target->access_level < $this->session->user_access_level)
     { if($this->Syn_model->delete($target->id))
 			    { $flash = 'Synoptique '.$target->libelle.' ('.$target->id.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/reload/dls');
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression du Synoptique '.$target->id ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression du synoptique ".$target->libelle.' ('.$target->id.')' );
     }
				redirect('admin/syn/index');
  }
/********************************************* Chargement de l'atelier ********************************************************/
	public function atelier ($id=NULL)
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    if (!isset($id)) $id=1;
    $syn = $this->Syn_model->get($id);
    $this->data["syn"] = $syn->libelle;
    $this->data["body_ready"] = "Charger_syn(".$id.")";

   	$ch = curl_init( "https://icons.abls-habitat.fr/icons/class_list" );
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $source = curl_exec($ch);
    $this->data["classes"] = json_decode($source);
    curl_close($ch);
    $this->template->admin_render('admin/syn/atelier', $this->data);
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function update_motifs ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $motifs = json_decode ( $this->input->raw_input_stream, TRUE );
    foreach ( $motifs as $key => $motif ):
     $param["libelle"]       = $motif["libelle"];
     $param["access_level"]  = $motif["access_level"];
     $param["posx"]          = $motif["posx"];
     $param["posy"]          = $motif["posy"];
     $param["angle"]         = $motif["angle"];
     $param["scale"   ]      = $motif["scale"];
     $param["def_color"]     = $motif["def_color"];
     $param["tech_id"]       = $motif["tech_id"];
     $param["acronyme"]      = $motif["acronyme"];
     $param["clic_tech_id"]  = $motif["clic_tech_id"];
     $param["clic_acronyme"] = $motif["clic_acronyme"];
     $this->Syn_model->update_motif ( $motif["id"], $param );
    endforeach;
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function delete_motif ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $motif = json_decode ( $this->input->raw_input_stream, TRUE );
    $this->Syn_model->delete_motif ( $motif["id"] );
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function add_motif ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    header("Content-Type: application/json; charset=UTF-8");

    $motif = json_decode ( $this->input->raw_input_stream, TRUE );
    $param["syn_id"]        = $motif["syn_id"];
    $param["bitctrl"]       = -1;
    $param["icone"]         = $motif["icone"];
    $param["libelle"]       = $motif["libelle"];
    $param["posx"]          = $motif["posx"];
    $param["posy"]          = $motif["posy"];
    $param["angle"]         = $motif["angle"];
    $param["scale"]         = $motif["scale"];
    $param["def_color"]     = $motif["def_color"];
    $param["tech_id"]       = $motif["tech_id"];
    $param["acronyme"]      = $motif["acronyme"];
    $param["clic_tech_id"]  = $motif["clic_tech_id"];
    $param["clic_acronyme"] = $motif["clic_acronyme"];
    $param["access_level"]  = $motif["access_level"];
    $new_id = $this->Syn_model->add_motif ( $param );
    $motif = $this->Syn_model->get_motif($new_id);
		  $output = array( "motif" => $motif	);
		  echo json_encode($motif);
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function update_liens ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $liens = json_decode ( $this->input->raw_input_stream, TRUE );
    foreach ( $liens as $key => $lien ):
     $param["src_posx"]         = $lien["src_posx"];
     $param["src_posy"]         = $lien["src_posy"];
     $param["dst_posx"]         = $lien["dst_posx"];
     $param["dst_posy"]         = $lien["dst_posy"];
     $param["stroke"]           = $lien["stroke"];
     $param["stroke_width"]     = $lien["stroke_width"];
     $param["stroke_dasharray"] = $lien["stroke_dasharray"];
     $param["stroke_linecap"]   = $lien["stroke_linecap"];
     $param["tech_id"]          = $lien["tech_id"];
     $param["acronyme"]         = $lien["acronyme"];
     $this->Syn_model->update_lien ( $lien["id"], $param );
    endforeach;
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function delete_lien ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $lien = json_decode ( $this->input->raw_input_stream, TRUE );
    $this->Syn_model->delete_lien ( $lien["id"] );
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function add_lien ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    header("Content-Type: application/json; charset=UTF-8");

    $lien = json_decode ( $this->input->raw_input_stream, TRUE );
    $param["syn_id"]       = $lien["syn_id"];
    $param["src_posx"]     = $lien["src_posx"];
    $param["src_posy"]     = $lien["src_posy"];
    $param["dst_posx"]     = $lien["dst_posx"];
    $param["dst_posy"]     = $lien["dst_posy"];
    $param["stroke"]       = $lien["stroke"];
    $param["stroke_width"] = $lien["stroke_width"];
    $param["stroke_dasharray"] = $lien["stroke_dasharray"];
    $param["stroke_linecap"]   = $lien["stroke_linecap"];
    $param["tech_id"]      = $lien["tech_id"];
    $param["acronyme"]     = $lien["acronyme"];
    $new_id = $this->Syn_model->add_lien ( $param );
    $lien = $this->Syn_model->get_lien($new_id);
		  $output = array( "lien" => $lien	);
		  echo json_encode($lien);
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function update_rectangles ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $rectangles = json_decode ( $this->input->raw_input_stream, TRUE );
    foreach ( $rectangles as $key => $rectangle ):
     $param["posx"]             = $rectangle["posx"];
     $param["posy"]             = $rectangle["posy"];
     $param["width"]            = $rectangle["width"];
     $param["height"]           = $rectangle["height"];
     $param["rx"]               = $rectangle["rx"];
     $param["ry"]               = $rectangle["ry"];
     $param["stroke"]           = $rectangle["stroke"];
     $param["stroke_width"]     = $rectangle["stroke_width"];
     $param["stroke_dasharray"] = $rectangle["stroke_dasharray"];
     $param["def_color"]        = $rectangle["def_color"];
     $param["tech_id"]          = $rectangle["tech_id"];
     $param["acronyme"]         = $rectangle["acronyme"];
     $this->Syn_model->update_rectangle ( $rectangle["id"], $param );
    endforeach;
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function delete_rectangle ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $lien = json_decode ( $this->input->raw_input_stream, TRUE );
    $this->Syn_model->delete_rectangle ( $rectangle["id"] );
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function add_rectangle ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    header("Content-Type: application/json; charset=UTF-8");
    $rectangle = json_decode ( $this->input->raw_input_stream, TRUE );
    $param["syn_id"]           = $rectangle["syn_id"];
    $param["posx"]             = $rectangle["posx"];
    $param["posy"]             = $rectangle["posy"];
    $param["width"]            = $rectangle["width"];
    $param["height"]           = $rectangle["height"];
    $param["rx"]               = $rectangle["rx"];
    $param["ry"]               = $rectangle["ry"];
    $param["stroke"]           = $rectangle["stroke"];
    $param["stroke_width"]     = $rectangle["stroke_width"];
    $param["stroke_dasharray"] = $rectangle["stroke_dasharray"];
    $param["def_color"]        = $rectangle["def_color"];
    $param["tech_id"]          = $rectangle["tech_id"];
    $param["acronyme"]         = $rectangle["acronyme"];
    $new_id = $this->Syn_model->add_rectangle ( $param );
    $rectangle = $this->Syn_model->get_rectangle($new_id);

		  $output = array( "rectangle" => $rectangle	);
		  echo json_encode($rectangle);
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function update_comments ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $comments = json_decode ( $this->input->raw_input_stream, TRUE );
    foreach ( $comments as $key => $comment ):
     $param["libelle"]      = $comment["libelle"];
     $param["posx"]         = $comment["posx"];
     $param["posy"]         = $comment["posy"];
     $param["angle"]        = $comment["angle"];
     $param["def_color"]    = $comment["def_color"];
     $param["font"]         = $comment["font"];
     $param["font_size"]    = $comment["font_size"];
     $this->Syn_model->update_comment ( $comment["id"], $param );
    endforeach;
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function delete_comment ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    $comment = json_decode ( $this->input->raw_input_stream, TRUE );
    $this->Syn_model->delete_comment ( $comment["id"] );
 	}
/********************************************* Mise à jour d'un synoptique ****************************************************/
	public function add_comment ()
  { if ( ! $this->wtd_auth->logged_in()) { redirect('auth/login', 'refresh'); }
    header("Content-Type: application/json; charset=UTF-8");
    $comment = json_decode ( $this->input->raw_input_stream, TRUE );
    $param["syn_id"]       = $comment["syn_id"];
    $param["libelle"]      = $comment["libelle"];
    $param["posx"]         = $comment["posx"];
    $param["posy"]         = $comment["posy"];
    $param["angle"]        = $comment["angle"];
    $param["def_color"]    = $comment["def_color"];
    $param["font"]         = $comment["font"];
    $param["font_size"]    = $comment["font_size"];
    $new_id = $this->Syn_model->add_comment ( $param );
    $comment = $this->Syn_model->get_comment($new_id);

		  $output = array( "comment" => $comment	);
		  echo json_encode($comment);
 	}
}
/*----------------------------------------------------------------------------------------------------------------------------*/
