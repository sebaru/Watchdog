<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Dls extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Dls_model');
    $this->load->model('Syn_model');

		  /* Title Dls :: Common */
    $this->admin_page_title->push('D.L.S');
    $this->data['pagetitle'] = "<h1>Modules D.L.S</h1>";
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Modules D.L.S', 'admin/dls');
  }

/******************************************************************************************************************************/
 function index()
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
			 /* Load Template */
			 $this->template->admin_render('admin/dls/index', $this->data);
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

 			$dlss = $this->Dls_model->get_all_dls($start, $length);
		  foreach($dlss->result() as $dls)
     { $dls_status = array( "<span class='label label-info'>Never compiled yet</span>",
                            "<span class='label label-danger'>Database Export failed</span>",
                            "<span class='label label-danger'>Error loading source file</span>",
                            "<span class='label label-danger'>Error loading log file</span>",
                            "<span class='label label-danger'>Syntax error</span>",
                            "<span class='label label-danger'>Error Fork GCC</span>",
                            "<span class='label label-warning'>OK with Warnings</span>",
                            "<span class='label label-success'>OK</span>",
                            "<span class='label label-danger'>Functions are missing<br>Need compiling again.</span>",
                            "<span class='label label-danger'>Error, plugin is setting bits he does not own.</span>",
                            "<span class='label label-danger'>error</span>"
                          );

       	$caret = '<div class="dropdown">'.
                 '<button class="btn btn-xs btn-primary dropdown-toggle" type="button" data-toggle="dropdown">'.
                 '<span class="caret"></span></button>'.
                 '<ul class="dropdown-menu">'.
                 '<li><a href='.site_url('admin/dls/edit/'.$dls->tech_id).'>'.
                     '<i class="fa fa-cog" style="color:blue"></i>Configurer</a></li>'.
                 '<li><a href='.site_url('admin/dls/sourceedit/'.$dls->tech_id).'>'.
                     '<i class="fa fa-code" style="color:blue"></i>Editer la source</a></li>'.
                 '<li><a href='.site_url('admin/dls/run/'.$dls->tech_id).'>'.
                     '<i class="fa fa-eye" style="color:blue"></i>Voir le RUN</a></li>'.
                 '<li><a href='.site_url('admin/mnemo/index/'.$dls->tech_id).'>'.
                     '<i class="fa fa-book" style="color:blue"></i>Editer les mnémos</a></li>'.
                 '<li class="divider"></li>'.
                 '<li><a href='.site_url('admin/dls/delete/'.$dls->tech_id).'>'.
                     '<i class="fa fa-times" style="color:red"></i>Supprimer ce D.L.S</a></li>'.
                 '</ul></div>';

        $data[] = array(	$caret,
                        ($dls->actif ? anchor('admin/dls/stop/'.$dls->tech_id, '<span class="label label-success">Activé</span>')
                                            : anchor('admin/dls/start/'. $dls->tech_id, '<span class="label label-default">Inactif</span>')),
 				                   '<a href='.site_url('admin/dls/sourceedit/'.$dls->tech_id).' data-toggle="tooltip"  style="cursor:pointer" title="Editer Source">
                        '.$dls->tech_id.'</a>',
                        $dls->package, $dls->ppage."/".$dls->page,
 				                   '<a href='.site_url('admin/dls/sourceedit/'.$dls->tech_id).' data-toggle="tooltip"  style="cursor:pointer" title="Editer Source">
                        '.$dls->shortname.'</a>', $dls->name,
                        $dls->compil_date,
                        $dls_status[$dls->compil_status],
                        $dls->nbr_ligne." / ".$dls->nbr_compil,
			                   );
		   }
		  $total_dls = $this->Dls_model->get_total_dls($this->session->user_access_level);

		  $output = array( "draw" => $draw,	"recordsTotal" => $total_dls,	"recordsFiltered" => $total_dls,	"data" => $data	);
		  echo json_encode($output);
		  exit();
  }

/******************************************************************************************************************************/
 function create($syn_id=NULL)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}
    if(!isset($syn_id)) {	redirect('auth', 'refresh');	}
    $this->data['syn'] = $syn = $this->Syn_model->get($syn_id);
    if (!isset($syn))  {	redirect('auth', 'refresh');	}
    if ($this->session->user_access_level < $syn->access_level) {	redirect('auth', 'refresh');	}

 			$this->admin_breadcrumbs->unshift(2, 'Ajouter un module DLS', 'admin/dls/create');
 			$this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

 			$this->form_validation->set_rules('tech_id','Tech_ID','required|is_unique[dls.tech_id]');
 			$this->form_validation->set_rules('shortname','Shortname','required');
 			$this->form_validation->set_rules('name','Libellé','required');

 			if($this->form_validation->run())
			  { $params = array( 'tech_id' => str_replace( " ", "_", str_replace("-", "_", $this->input->post('tech_id'))),
					                   'name' => $this->input->post('name'),
					                   'shortname' => $this->input->post('shortname'),
					                   'package' => $this->input->post('package'),
					                   'syn_id' => $syn->id,
					                   'actif' => 'FALSE',
				                  );

       if ($this->input->post('package'))	{	$data['package'] = $this->input->post('package');	}
                                     else {	$data['package'] = 'custom';	}

				   $dls_id = $this->Dls_model->add($params);
       $flash = 'Le dls <strong>'.$this->input->post('tech_id').'</strong> ('.$dls_id.') a été ajouté.';
				   $this->session->set_flashdata('flash_message', $flash);
       $this->wtd_log->add($flash);
       $this->wtd_webservice->send('/process/reload/dls');
       redirect('admin/syn/index');
     }
    else $this->session->set_flashdata('flash_error', validation_errors() );

    $this->data['tech_id'] = array(	'name'  => 'tech_id',
	                                   'id'    => 'tech_id',
	                                   'type'  => 'text',
                                    'class' => 'form-control',
	                                   'value' => $this->form_validation->set_value('tech_id', $this->input->post('tech_id'))
                                  );

    $this->data['name'] = array(	'name'  => 'name',
                                 'id'    => 'name',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('name', $this->input->post('name'))
                               );

    $this->data['shortname'] = array(	'name'  => 'shortname',
	                                     'id'    => 'shortmane',
	                                     'type'  => 'text',
                                      'class' => 'form-control',
	                                     'value' => $this->form_validation->set_value('shortname', $this->input->post('shortname'))
                                    );

    $this->data['package'] = array(	'name'  => 'package',
                                    'id'    => 'name',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('name', $this->input->post('package'))
                                  );

				$this->template->admin_render('admin/dls/add', $this->data);
  }
/******************************************************************************************************************************/
	public function delete($tech_id)
 	{ if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/index');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->delete($target_dls->tech_id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') supprimé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/dls/delete?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', 'Erreur de suppression du Module D.L.S' ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de suppression du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/index');
	 }
/******************************************************************************************************************************/
	public function start($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/index');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->start($target_dls->tech_id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') activé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/dls/start?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', "Erreur d'activation du Module D.L.S" ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative d'activation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/index');
	 }
/******************************************************************************************************************************/
	public function stop($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/index');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
     { if($this->Dls_model->stop($target_dls->tech_id))
 			    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') désactivé';
          $this->session->set_flashdata('flash_message', $flash );
          $this->wtd_webservice->send('/dls/stop?id='.$target_dls->id);
          $this->wtd_log->add($flash);
        }
       else
 			    { $this->session->set_flashdata('flash_error', "Erreur de désactivation du Module D.L.S" ); }
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative désactivation du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/index');
	 }
/******************************************************************************************************************************/
	public function acquit($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/run');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
	    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') acquitté';
       $this->wtd_webservice->send('/dls/acquit?id='.$target_dls->id);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative d'acquit du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/run');
	 }
/******************************************************************************************************************************/
	public function debug($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/run');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
	    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') DEBUG ON';
       $this->wtd_webservice->send('/dls/debug?id='.$target_dls->id);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de debug du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/run');
	 }
/******************************************************************************************************************************/
	public function undebug($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $target_dls = $this->Dls_model->get($tech_id);
    if (!isset($target_dls))
     { $this->session->set_flashdata('flash_error', 'Module D.L.S '.$tech_id.' inconnu' );
       redirect('admin/dls/run');
     }
    if ($target_dls->access_level < $this->session->user_access_level)
	    { $flash = 'Module D.L.S ('.$target_dls->tech_id.', '.$target_dls->name.') DEBUG OFF';
       $this->wtd_webservice->send('/dls/undebug?id='.$target_dls->id);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de undebug du module D.L.S ".
                           $target_dls->id." (".$target_dls->tech_id.", ".$target_dls->name.")");
     }
    redirect('admin/dls/run');
	 }
/******************************************************************************************************************************/
 function edit($tech_id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

			 // check if the dls exists before trying to edit it
			 $this->data['dls'] = $dls = $this->Dls_model->get($tech_id);
 			if(!isset($dls))
     { $this->session->set_flashdata('flash_error', "Ce D.L.S n'existe pas");
       redirect('admin/dls/index');
     }

 			if ($this->session->user_access_level<$dls->access_level)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

    $this->admin_breadcrumbs->unshift(2, 'Editer un module', 'admin/dls/edit/'.$tech_id);
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

				$this->form_validation->set_rules('tech_id','Tech ID','required');
				$this->form_validation->set_rules('shortname','Shortname','required');

   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'tech_id'    => $this->input->post('tech_id'),
			                     'name'       => $this->input->post('name'),
			                     'shortname'  => $this->input->post('shortname'),
			                     'package'    => $this->input->post('package'),
			                     'actif'         => (null !== $this->input->post('actif') ? TRUE : FALSE),
                 					);

	  				$this->Dls_model->update($tech_id,$params);
  					$this->session->set_flashdata('flash_message','Le dls <strong>'.$this->input->post('tech_id').'</strong>/'.
                                     $this->input->post('shortname').' a été modifié.');
       $this->wtd_log->add("Mise à jour du DLS ".$dls->id." (".$this->input->post('tech_id').") ");
       redirect('admin/dls/index');
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

		  $this->data['tech_id'] = array(	'name'  => 'tech_id',
			                                 'id'    => 'tech_id',
			                                 'type'  => 'text',
                                    'class' => 'form-control',
			                                 'value' => $this->form_validation->set_value('tech_id', $dls->tech_id)
		                                );
		  $this->data['shortname'] = array(	'name'  => 'shortname',
                                      'id'    => 'shortname',
                                      'type'  => 'text',
                                      'class' => 'form-control',
                                      'value' => $this->form_validation->set_value('shortname', $dls->shortname)
                                    );
		  $this->data['name'] = array(	'name'  => 'name',
                                 'id'    => 'name',
                                 'type'  => 'text',
                                 'class' => 'form-control',
                                 'value' => $this->form_validation->set_value('name', $dls->name)
                               );

		  $this->data['package'] = array(	'name'  => 'package',
                                    'id'    => 'package',
                                    'type'  => 'text',
                                    'class' => 'form-control',
                                    'value' => $this->form_validation->set_value('package', $dls->package)
                                  );

		  $this->data['actif'] = array(	'name'  => 'actif',
                                  'id'    => 'actif',
                                 'checked' => $this->form_validation->set_value('actif', $dls->actif)
                                );

        /* Load Template */
		  $this->template->admin_render('admin/dls/edit', $this->data);
 	}
/******************************************************************************************************************************/
 function sourceedit($tech_id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

    $this->admin_breadcrumbs->unshift(2, 'Editer une source D.L.S', 'admin/dls/sourceedit');
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

			 // check if the dls exists before trying to edit it
			 $this->data['dls'] = $dls = $this->Dls_model->get($tech_id);
 			if(!isset($dls))
     { $this->session->set_flashdata('flash_error', "Ce D.L.S n'existe pas");
       redirect('admin/dls/index');
     }

				$this->form_validation->set_rules('sourcecode','Code Source','required');

   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'sourcecode' => $this->input->post('sourcecode'),
                 					);

	  				$this->Dls_model->update($dls->tech_id,$params);
  					$this->session->set_flashdata('flash_message','Le dls <strong>'.$dls->tech_id.'</strong>/'.
                                     $dls->shortname.' a été modifié.');
       $this->wtd_log->add("Mise à jour et compilation de la source DLS ".$dls->id." (".$dls->tech_id.") ");
       $this->wtd_webservice->compil($dls->id);
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

    $this->data['error_log'] = $this->Dls_model->get_error_log($dls->tech_id);
    $this->data['sourcecode'] = array(	'name'  => 'sourcecode',
                                       'id'    => 'sourcecode',
                                       'type'  => 'textarea',
                                       'class' => 'form-control',
                                       'value' => $this->form_validation->set_value('sourcecode', $dls->sourcecode)
                                     );
		  $this->template->admin_render('admin/dls/sourceedit', $this->data);
 	}
/******************************************************************************************************************************/
 function download_package($tech_id=null)
  { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');	}

 			if ($this->session->user_access_level<6)
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       redirect('admin/dls/index');
     }

    $this->admin_breadcrumbs->unshift(2, 'Editer une source D.L.S', 'admin/dls/sourceedit');
	   $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();

			 // check if the dls exists before trying to edit it
			 $this->data['dls'] = $dls = $this->Dls_model->get($tech_id);
 			if(!isset($dls))
     { $this->session->set_flashdata('flash_error', "Ce D.L.S n'existe pas");
       redirect('admin/dls/index');
     }

 			$this->form_validation->set_rules('sourcecode','Code Source','required');

   	if($this->form_validation->run() == TRUE)
	    { $params = array(	'sourcecode' => $this->input->post('sourcecode'),
                 					);

	  				$this->Dls_model->update($dls->tech_id,$params);
  					$this->session->set_flashdata('flash_message','Le dls <strong>'.$dls->tech_id.'</strong>/'.
                                     $dls->shortname.' a été modifié.');
       $this->wtd_log->add("Mise à jour et compilation de la source DLS ".$dls->id." (".$dls->tech_id.") ");
       $this->wtd_webservice->compil($dls->id);
		 		}
    else
     { $this->session->set_flashdata('flash_error', validation_errors() ); }

    $ch = curl_init( "https://packages.abls-habitat.fr/".$dls->package.".dls" );
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $new_source = curl_exec($ch);
    curl_close($ch);

    $this->data['dls']->sourcecode = $new_source;

    $this->data['error_log'] = $this->Dls_model->get_error_log($dls->tech_id);
    $this->data['sourcecode'] = array(	'name'  => 'sourcecode',
                                       'id'    => 'sourcecode',
                                       'type'  => 'textarea',
                                       'class' => 'form-control',
                                       'value' => $this->form_validation->set_value('sourcecode', $dls->sourcecode)
                                     );
		  $this->template->admin_render('admin/dls/sourceedit', $this->data);
 	}
/******************************************************************************************************************************/
	function run($tech_id=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
			 if ($tech_id==NULL)
     { $this->data['dlss'] = $this->wtd_webservice->get_primary("/dls/list");                     /* Get all running conf DLS */
       $this->template->admin_render('admin/dls/run', $this->data);                                          /* Load Template */
     }
    else
     { $this->data['plugin'] = $this->wtd_webservice->get_primary("/dls/run/".$tech_id);          /* Get all running conf DLS */
       $this->template->admin_render('admin/dls/plugin_run', $this->data);                                   /* Load Template */
     }
 	}
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
