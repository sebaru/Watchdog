<?php namespace App\Controllers;

class Home extends BaseController
{
	public function index()
	{
		return view('welcome_message');
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
