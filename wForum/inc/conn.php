<?php 
/*
require 'DB.php';
$dsn = "mysql://$dbuser:$dbpasswd@$dbhost/$dbname";

$conn = DB::connect($dsn);
if (DB::isError($conn)) { die ($conn->getMessage()); }
function CloseDatabase()
{
	global $conn;
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
	
	function my_mysql($dbhost, $dbuser, $dbpasswd, $dbname) {
		@$this->my_conn = @mysql_connect($dbhost, $dbuser, $dbpasswd) or die("MYSQL connection failure.");
		@mysql_select_db($dbname) or die("MYSQL: cannot select database.");
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

$conn = new my_mysql($dbhost, $dbuser, $dbpasswd, $dbname);
function CloseDatabase()
{
	global $conn;
	$conn->close();
	$conn = false;
}
?>
