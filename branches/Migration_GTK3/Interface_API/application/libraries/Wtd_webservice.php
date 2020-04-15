<?php
defined('BASEPATH') OR exit('No direct script access allowed');

 class Wtd_webservice
  { protected $CI;

    public function __construct()
     {	$this->CI =& get_instance(); }

   	public function status()
	    { $ch = curl_init( $this->CI->config->item('web_service_url').'/status' );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $result = json_decode(curl_exec($ch))->Status;
       curl_close($ch);
		     return $result;
	    }
	
   	public function compil($id)
     { $ch = curl_init( $this->CI->config->item('web_service_url')."/compil?id=".$id );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       curl_exec($ch);
       curl_close($ch);
     }
	
   	public function send($uri)
	    { $this->CI->wtd_log->add( 'Call to '.$this->CI->config->item('web_service_url').$uri );
       $ch = curl_init( $this->CI->config->item('web_service_url').$uri );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       curl_exec($ch);
       curl_close($ch);
	    }
   	public function get($uri)
	    { $this->CI->wtd_log->add( 'Call to '.$this->CI->config->item('web_service_url').$uri );
       $ch = curl_init( $this->CI->config->item('web_service_url').$uri );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $result = json_decode(curl_exec($ch));
       error_log( "result ".$this->CI->config->item('web_service_url').$uri );
       foreach ($result as $nom => $details)
        { foreach ($details as $key => $detail)
           { error_log("$nom : $key => $detail\n"); }
        }
       curl_close($ch);
		     return($result);
	    }

	 }
