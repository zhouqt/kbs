<?php

$needlogin=0;
require("inc/funcs.php");
setStat("更换界面");

do_changeStyle();

if (!isset($_SERVER["HTTP_REFERER"]) || ( $_SERVER["HTTP_REFERER"]=="") )
{
	header("Location: index.php");
} else {
	header("Location: ".$_SERVER["HTTP_REFERER"]);
} 

function do_changeStyle()
{
	if (!isset($_GET["style"])) { //ToDo: 是否要检查 style 是不是合法？
		foundErr("非法的参数。");
	} 
	setcookie("style",$_GET["style"],time()+ 604800 ); //7*24*60*60
} 
?>