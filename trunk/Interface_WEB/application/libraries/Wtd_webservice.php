<?php
defined('BASEPATH') OR exit('No direct script access allowed');

 class Wtd_webservice
  { protected $CI;

    public function __construct()
     {	$this->CI =& get_instance();
     }

   	public function instance()
	    { if (isset($this->CI->session->instance)) return($this->CI->session->instance);
       else return($this->CI->config->item('web_service_host'));
	    }

   	public function get_ws_url()
	    { return("http://".$this->CI->session->username.":".$this->CI->session->user_password."@".$this->instance().":5560"); }

   	public function get_primary_ws_url()
	    { return("http://".$this->CI->session->username.":".$this->CI->session->user_password."@".$this->CI->config->item('web_service_host').":5560"); }

   	public function set_instance($instance)
	    { $this->CI->session->set_userdata( "instance", $instance );
       $this->CI->wtd_log->add("Setting target instance to ".$this->instance());
	    }

   	public function status()
	    { $ch = curl_init( $this->CI->wtd_webservice->get_ws_url().'/status' );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $result = json_decode(curl_exec($ch));
       curl_close($ch);
		     return $result;
	    }

   	public function master_status()
	    { $ch = curl_init( $this->CI->wtd_webservice->get_primary_ws_url().'/status' );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $result = json_decode(curl_exec($ch));
       curl_close($ch);
		     return $result;
	    }

   	public function compil($id)
     { $ch = curl_init( $this->CI->wtd_webservice->get_primary_ws_url()."/dls/compil?id=".$id );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       curl_exec($ch);
       curl_close($ch);
     }

   	public function send($uri)
	    { $this->CI->wtd_log->add( 'Call to '.$uri );
       $ch = curl_init( $this->CI->wtd_webservice->get_ws_url().$uri );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       curl_exec($ch);
       curl_close($ch);
	    }
   	public function get_local($uri)
	    { $this->CI->wtd_log->add( 'Call to '.$uri );
       $ch = curl_init( $this->CI->wtd_webservice->get_ws_url().$uri );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $source = curl_exec($ch);
       $result = json_decode($source);
       error_log( "result ".$this->CI->wtd_webservice->get_ws_url().$uri. " = ".$source );
       if ($result==NULL) error_log("Json decode error:". json_last_error_msg());
       curl_close($ch);
		     return($result);
	    }
   	public function get_primary($uri)
	    { $this->CI->wtd_log->add( 'Call to '.$uri );
       $ch = curl_init( $this->CI->wtd_webservice->get_primary_ws_url().$uri );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $source = curl_exec($ch);
       $result = json_decode($source);
       if ($result==NULL) error_log("Json decode error:". json_last_error_msg());
       curl_close($ch);
		     return($result);
	    }
   	public function post( $uri, $param )
	    { $this->CI->wtd_log->add( 'Call to '.$uri );
       $ch = curl_init( $this->CI->wtd_webservice->get_ws_url().$uri );
       curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
       $json = json_encode($param);
       curl_setopt($ch, CURLOPT_POSTFIELDS, $json );
       curl_setopt($ch, CURLOPT_HTTPHEADER, array( 'Content-Type: application/json',
                                                   'Content-Length: ' . strlen($json))
                                                 );
		     curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       $source = curl_exec($ch);
       curl_close($ch);
       if ($source!=FALSE)
        { $result = json_decode($source);
          /*log_message( 'debug', "result ".$this->CI->wtd_webservice->get_ws_url().$uri. " = ".$source );*/
		        return($result);
        }
       return(FALSE);
	    }
	 }
/*----------------------------------------------------------------------------------------------------------------------------*/
