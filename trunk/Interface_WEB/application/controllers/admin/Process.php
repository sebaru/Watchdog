<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Process extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Config_model');
    $this->data['instances'] = $this->Config_model->get_instances();                                     /* Get all instances */

		  /* Title Dls :: Common */
    $this->admin_page_title->push('Processus');
    $this->data['pagetitle'] = "<h1>Listes des processus sur <strong>".$this->wtd_webservice->instance()."</strong></h1>";
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Processus', 'admin/process');
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	public function set_instance()
	 { $this->wtd_webservice->set_instance( $this->data["instances"][$this->input->post("instance")]->instance_id );
	   redirect('admin/process');
  }
/******************************************************************************************************************************/
	public function start($thread=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    if (6 <= $this->session->user_access_level)
     { $flash = 'Processus ('.$thread.') activé';
       $this->session->set_flashdata('flash_message', $flash );
       $this->Config_model->set($this->wtd_webservice->instance(), $thread, "enable", "true");
       $this->wtd_webservice->send('/process/start/'.$thread);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative d'activation du processus ". $thread );
     }
    redirect('admin/process/index');
	 }
/******************************************************************************************************************************/
	public function stop($thread=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    if (6 <= $this->session->user_access_level)
     { $flash = 'Processus ('.$thread.') désactivé';
       $this->session->set_flashdata('flash_message', $flash );
       $this->Config_model->set($this->wtd_webservice->instance(), $thread, "enable", "false");
       $this->wtd_webservice->send('/process/stop/'.$thread);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de désactivation du processus ". $thread );
     }
    redirect('admin/process/index');
	 }
/******************************************************************************************************************************/
	public function reload($thread=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    if ($target_dls->access_level < $this->session->user_access_level)
     { $flash = 'Processus ('.$thread.') reloading';
       $this->session->set_flashdata('flash_message', $flash );
       $this->wtd_webservice->send('/process/reload/'.$thread);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de reload du processus ". $thread );
     }
    redirect('admin/process/index');
	 }
/******************************************************************************************************************************/
	public function debug($thread=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    if ($target_dls->access_level < $this->session->user_access_level)
     { $flash = 'Processus ('.$thread.') debug activé';
       $this->session->set_flashdata('flash_message', $flash );
       $this->Config_model->set($this->wtd_webservice->instance(), $thread, "debug", "true");
       $this->wtd_webservice->send('/process/debug/'.$thread);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de debug du processus ". $thread );
     }
    redirect('admin/process/index');
	 }
/******************************************************************************************************************************/
	public function undebug($thread=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    if ($target_dls->access_level < $this->session->user_access_level)
     { $flash = 'Processus ('.$thread.') debug désactivé';
       $this->session->set_flashdata('flash_message', $flash );
       $this->Config_model->set($this->wtd_webservice->instance(), $thread, "debug", "false");
       $this->wtd_webservice->send('/process/undebug/'.$thread);
       $this->wtd_log->add($flash);
		   }
    else
     { $this->session->set_flashdata('flash_error', 'Privilèges insuffisants' );
       $this->wtd_log->add("Tentative de undebug du processus ". $thread );
     }
    redirect('admin/process/index');
	 }
/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
    $this->data['processus'] = $this->wtd_webservice->get_local("/process/list")->Process;    /* Get all running process */
    $this->template->admin_render('admin/process/index', $this->data);                                       /* Load Template */
 	}
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
