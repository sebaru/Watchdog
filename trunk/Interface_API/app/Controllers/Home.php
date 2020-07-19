<?php namespace App\Controllers;

class Home extends BaseController
{
	public function index()
	 {	if ( session()->get('user') === NULL )         { return redirect()->to('/auth/login'); }
    if ( session()->get('user')->access_level>=6 ) { return redirect()->to('/tech/dashboard'); }
    return redirect()->to('/home/synmobile/1');
	 }

/******************************************************************************************************************************/
 public function archive()
  { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }

    echo view('Home/header');
    echo view('Home/archive');
    echo view('Home/footer');

  }

/******************************************************************************************************************************/
 public function synmobile()
  { if ( session()->get('user') === NULL )        { return redirect()->to('/auth/login'); }

    echo view('Home/header');
    echo view('Home/synmobile');
    echo view('Home/footer');

   }
	//--------------------------------------------------------------------

}
