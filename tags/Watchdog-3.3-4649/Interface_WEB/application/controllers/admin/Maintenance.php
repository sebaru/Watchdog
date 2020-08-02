<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Maintenance extends Admin_Controller{
 function __construct()
  { parent::__construct();
    $this->load->model('Msg_model');

		  /* Title Dls :: Common */
    $this->admin_page_title->push('Maintenance');
    /* Breadcrumbs :: Common */
    $this->admin_breadcrumbs->unshift(1, 'Maintenance', 'admin/maintenance');
    $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();                                              /* Breadcrumbs */
  }

/******************************************************************************************************************************/
	public function clear_histo()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    $this->Msg_model->clear_histo();
    $flash = 'Messages archivés !';
    $this->session->set_flashdata('flash_message', $flash );
    $this->wtd_log->add($flash);
    redirect('admin/maintenance/index');
	 }
/******************************************************************************************************************************/
	public function log($niveau=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    switch($niveau)
     { case 'debug'   : $this->wtd_webservice->send( '/log/debug' ); break;
       case 'info'    : $this->wtd_webservice->send( '/log/info' ); break;
       case 'notice'  : $this->wtd_webservice->send( '/log/notice' ); break;
       case 'warning' : $this->wtd_webservice->send( '/log/warning' ); break;
       case 'error'   : $this->wtd_webservice->send( '/log/error' ); break;
     }
    $flash = 'Log level changed !';
    $this->session->set_flashdata('flash_message', $flash );
    $this->wtd_log->add($flash);
    redirect('admin/maintenance/index');
	 }
/******************************************************************************************************************************/
	public function archive($mode=NULL)
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth', 'refresh');		}

    switch($mode)
     { case 'clear'  : $this->wtd_webservice->send( '/archive/clear' ); $flash = 'Archive Cleared !'; break;
       case 'purge'  : $this->wtd_webservice->send( '/archive/purge' ); $flash = 'Archive Purge in progress !'; break;
       case 'testdb' : $this->wtd_webservice->send( '/archive/testdb' ); $flash = 'Archive DB Connection test sent !'; break;
       default: $flash = "Commande inconnue";
     }
    $this->session->set_flashdata('flash_message', $flash );
    $this->wtd_log->add($flash);
    redirect('admin/maintenance/index');
	 }
/******************************************************************************************************************************/
	function index()
	 { if ( ! $this->wtd_auth->logged_in() ) {	redirect('auth/login', 'refresh');	}
			 $this->data['breadcrumb'] = $this->admin_breadcrumbs->show();
    $this->template->admin_render('admin/maintenance/index', $this->data);                                   /* Load Template */
 	}
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
