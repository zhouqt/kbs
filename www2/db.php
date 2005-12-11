<?php
/**
 * this file defined a database class base on MySQL
 * @author: windinsn May 28,2004
 */

class BbsDb
{
var $host;  // database host name
var $port;  // database port
var $user;  // database user
var $pass;  // database password
var $db;    // database name
var $link;  // database query resource link
var $arrays;// return arrays
var $rows;  // return rows
var $nums;  // numbers of returns
var $err;   // database error string
var $sql;   // SQL String
    
function BbsDb($db='') {
    $this->host = bbs_sysconf_str("MYSQLHOST");
    $this->user = bbs_sysconf_str("MYSQLUSER");
    $this->pass = bbs_sysconf_str("MYSQLPASSWORD");
    $this->db   = $db?$db:bbs_sysconf_str("MYSQLSMSDATABASE");
    
    $this->link = mysql_connect($this->host,$this->user,$this->pass);
    if (!$this->link) {
        $this->err = mysql_error();
        return false;
    }
    if (!mysql_select_db($this->db,$this->link)) {
        $this->err = mysql_error();
        return false;
    }
    return true;
}

function get_insert_id() {
    return mysql_insert_id($this->link);    
}

function query($sql,$type=0) {
    $this->sql = $sql;
    $result = mysql_query($this->sql,$this->link);
    if (!$result) {
        $this->err = mysql_error();
        return false;
    }
    switch($type) {
        case 1: //return arrays
            $this->arrays = array();
            while($rows = mysql_fetch_array($result))
                $this->arrays[] = $rows;
            $this->nums = mysql_num_rows($result);
            mysql_free_result($result);
            break;
        case 2: //return rows
            $this->rows = array();
            while($rows = mysql_fetch_row($result))
                $this->rows[] = $rows;
            $this->nums = mysql_num_rows($result);
            mysql_free_result($result);
            break;
        default:    
    }
    return true;
}

function quit() {
   mysql_close($this->link);
}
    
}
?>