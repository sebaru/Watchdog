<?php
/*
 * Generated by CRUDigniter v3.2
 * www.crudigniter.com
 */

class Archive_model extends CI_Model
 { public function __construct()
    { parent::__construct();
      $this->archdb = $this->load->database('ArchiveDB', TRUE);
    }

/******************************************************************************************************************************/
 function get_ea_hour($tech_id, $acronyme)
  { $this->archdb->select("FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV 60)*60) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne");
		  $this->archdb->from('histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    log_message ( 'debug', 'table = '.'histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    $this->archdb->where('date_time>=','NOW() - INTERVAL 4 HOUR',FALSE);
    $this->archdb->group_by('date');
    $this->archdb->order_by('date');
    return $this->archdb->get()->result();
  }
/******************************************************************************************************************************/
 function get_ea_day($tech_id, $acronyme)
  { $this->archdb->select("FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV 450)*450) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne");
		  $this->archdb->from('histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    log_message ( 'debug', 'table = '.'histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    $this->archdb->where('date_time>=','NOW() - INTERVAL 2 DAY',FALSE);
    $this->archdb->group_by('date');
    $this->archdb->order_by('date');
    return $this->archdb->get()->result();
  }
/******************************************************************************************************************************/
 function get_ea_week($tech_id, $acronyme)
  { $this->archdb->select("FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV 3600)*3600) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne");
		  $this->archdb->from('histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    log_message ( 'debug', 'table = '.'histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    $this->archdb->where('date_time>=','NOW() - INTERVAL 2 WEEK',FALSE);
    $this->archdb->group_by('date');
    $this->archdb->order_by('date');
    return $this->archdb->get()->result();
  }
/******************************************************************************************************************************/
 function get_ea_month($tech_id, $acronyme)
  { $this->archdb->select("FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV 43200)*43200) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne");
		  $this->archdb->from('histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    log_message ( 'debug', 'table = '.'histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    $this->archdb->where('date_time>=','NOW() - INTERVAL 9 WEEK',FALSE);
    $this->archdb->group_by('date');
    $this->archdb->order_by('date');
    return $this->archdb->get()->result();
  }
/******************************************************************************************************************************/
 function get_ea_year($tech_id, $acronyme)
  { $this->archdb->select("FROM_UNIXTIME((UNIX_TIMESTAMP(date_time) DIV 86400)*86400) AS date, COALESCE(ROUND(AVG(valeur),3),0) AS moyenne");
		  $this->archdb->from('histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    log_message ( 'debug', 'table = '.'histo_bit_'.strtoupper($tech_id).'_'.strtoupper($acronyme) );
    $this->archdb->where('date_time>=','NOW() - INTERVAL 13 MONTH',FALSE);
    $this->archdb->group_by('date');
    $this->archdb->order_by('date');
    return $this->archdb->get()->result();
  }
}
/*----------------------------------------------------------------------------------------------------------------------------*/