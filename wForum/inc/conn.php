<?php 
/* 数据库连接基础文件。$conn 为当前数据库连接类。如果数据库连接出现错误，$conn 为 false。 */

/*
require 'DB.php';
$dsn = "mysql://$dbuser:$dbpasswd@$dbhost/$dbname";

if (DB_ENABLED) {
	$conn = DB::connect($dsn);
	if (DB::isError($conn)) {
		//die ($conn->getMessage());
		$conn = false;
	}
} else {
	$conn = false;
}
function CloseDatabase()
{
	global $conn;
	if ($conn === false) return;
	$conn->disconnect();
	//unset($conn); //这是不对的，因为 function 内 unset 不会 destroy 全局变量 - atppp
	$conn = false;
}
*/
/* 以上是通用数据库连接，会降低速度，如果是使用 mysql 数据库，直接用下面的代码 - atppp */


/* MYSQL 数据库直接支持 - atppp */
define('DB_FETCHMODE_ASSOC', MYSQL_ASSOC);

class my_result {
	var $result_link;
	
	function my_result($rl) {
		$this->result_link = $rl;
	}
	
	function numRows() {
		return mysql_num_rows($this->result_link);
	}
	
	function fetchRow($mode) {
		return mysql_fetch_array($this->result_link, $mode);		
	}
	
	function free() {
		mysql_free_result($this->result_link);
	}
}

class my_mysql {
	var $my_conn;
	var $my_err;
	
	function my_mysql($dbhost, $dbuser, $dbpasswd, $dbname) {
		$this->my_err = false;
		if (!function_exists('mysql_connect'))  {
		    $this->my_err = true;
		} else {
    		@$this->my_conn = @mysql_connect($dbhost, $dbuser, $dbpasswd) or $this->my_err = true;
    		if (!$this->my_err) @mysql_select_db($dbname) or $this->my_err = true;
    	}
	}
	
	function query($sql) {
		return new my_result(mysql_query($sql, $this->my_conn));
	}
	
	function close() {
		mysql_close($this->my_conn);
	}
	
	function getRow($sql, $mode) {
		$re = $this->query($sql);
		return $re->fetchRow($mode);
	}
}

if (DB_ENABLED) {
	$conn = new my_mysql($dbhost, $dbuser, $dbpasswd, $dbname);
	if ($conn->my_err) $conn = false;
} else {
	$conn = false;
}
function CloseDatabase()
{
	global $conn;
	$conn->close();
	$conn = false;
}
?>
