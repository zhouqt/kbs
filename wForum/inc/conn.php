<?php 
require 'DB.php';
$host='aka.com.cn';
$user='aka';
$passwd='aA3$;G(~cjKK';
$dbname='AKA';
$dsn = "mysql://$user:$passwd@$host/$dbname";

$conn = DB::connect($dsn);
if (DB::isError($conn)) { die ($conn->getMessage()); }
function CloseDatabase()
{
	global $conn;
	$conn->disconnect();
	unset($conn);
} 
?>
