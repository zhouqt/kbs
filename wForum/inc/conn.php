<?php 
require 'DB.php';
$host='localhost';
$user='root';
//$passwd='phpsql';
$dbname='wbbs';
//$dsn = "mysql://$user:$passwd@$host/$dbname";
$dsn = "mysql://$user@$host/$dbname";

$conn = DB::connect($dsn);
if (DB::isError($conn)) { die ($conn->getMessage()); }
function CloseDatabase()
{
	global $conn;
	$conn->disconnect();
	unset($conn);
} 
?>
