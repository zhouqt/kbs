<?php
function bbs_create_activation()
{
	$chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	mt_srand((double)microtime()*1000000*getmypid());
	$activation = "";
	while(strlen($activation) < ACTIVATIONLEN)
		$activation .= substr($chars,(mt_rand()%strlen($chars)),1);
	return $activation;
}

function bbs_reg_newactivation($activation,$reg_email)
{
	return "0".$activation.$reg_email;
}

function bbs_reg_haveactivated($activation)
{
	$activate_state = intval($activation[0]);
	if($activate_state==1)
		return TRUE;
	else
		return FALSE;
}

function bbs_reg_getactivationcode($activation)
{
	$activation_code = substr($activation,1,ACTIVATIONLEN);
	return $activation_code;	
}

function bbs_reg_getactivationemail($activation)
{
	$activation_email = substr($activation,1+ACTIVATIONLEN);
	return $activation_email;	
}

function bbs_reg_successactivation($activation)
{
	$activation_new = "1".substr($activation,1,strlen($activation)-1);
	return $activation_new;
}

?>