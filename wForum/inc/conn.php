<?php 
require 'DB.php';
$host='localhost';
$user='wForum';
$passwd='thisISwForum';
$dbname='wForum';
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
