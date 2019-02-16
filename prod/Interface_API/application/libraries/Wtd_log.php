<?php
defined('BASEPATH') OR exit('No direct script access allowed');

class Wtd_log
 {	public function __construct()
    {	
    } 

   public function __get($var)
    { return get_instance()->$var; }

/******************************************************************************************************************************/
   public function add($message)
 	  { if (isset($this->session->username))
       { $data = array( 'username'     => $this->session->username,
                        'access_level' => $this->session->user_access_level,
                        'message'      => $message );
       }
      else
       { $data = array( 'username'     => 'none',
                        'access_level' => 0,
                        'message'      => $message );
       }
 		   $return = $this->db->insert('audit_log', $data );
	     return $return;
   	}
/******************************************************************************************************************************/
   public function get_all($start, $length)
    { $this->db->select("*");
   		 $this->db->from('audit_log');
      $this->db->where('access_level<=', $this->session->user_access_level);
      if ($length != 0) { $this->db->limit($length,$start); }
		    return $this->db->get();
    }
/******************************************************************************************************************************/
  	public function get_total()
    { $this->db->select("*");
   		 $this->db->from('audit_log');
      $this->db->where('access_level<=', $this->session->user_access_level);
	     $query = $this->db->select("COUNT(*) as num")->get();
  		  $result = $query->row();
  		  if(isset($result)) return $result->num;
		    return 0;
	   }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
