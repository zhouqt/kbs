<?php

$wwwparams = $_COOKIE["WWWPARAMS"];

function bbs_style_getstyle()
{
	global $wwwparams;
	return $wwwparams[0];	
}


?>