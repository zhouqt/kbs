<?php

/*
** this file define some functions used in personal corp.
** @id:windinsn Nov 19,2003
*/
require("funcs.php");
$db["HOST"]=bbs_sysconf_str("MYSQLHOST");
$db["USER"]=bbs_sysconf_str("MYSQLUSER");
$db["PASS"]=bbs_sysconf_str("MYSQLPASSWORD");
$db["NAME"]=bbs_sysconf_str("MYSQLSMSDATABASE");

/*
** personal corp. configure start
** LIST: user number in pc.php
** HOME: bbs home directory
** ETEMS: etems in xml file
** SITE: site address,used in xml file
** BOARD: whose manager can manage anyone's personal corp.
*/
$pcconfig["LIST"] = 20;
$pcconfig["HOME"] = BBS_HOME;
$pcconfig["ETEMS"] = 20;
$pcconfig["NEWS"] = 20;
$pcconfig["SITE"] = "smth.org";
$pcconfig["BOARD"] = "SMTH_blog";
$pcconfig["SEARCHFILTER"] = " 的";
$pcconfig["SEARCHNUMBER"] = 10;
/* personal corp. configure end */

$brdarr = array();
$pcconfig["BRDNUM"] = bbs_getboard($pcconfig["BOARD"], $brdarr);

if(!$currentuser["userid"])
		$currentuser["userid"] = "guest";

function pc_html_init($charset,$title="",$otherheader="",$style="",$bkimg="")
{
	global $_COOKIE;
	global $cachemode;
	global $currentuser;
	if ($cachemode=="") 
	{
		cache_header("no-cache");
		Header("Cache-Control: no-cache");
    	}
?>
<?xml version="1.0" encoding="<?php echo $charset; ?>"?>
<!DOCTYPE html
     PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=<?php echo $charset; ?>"/>
<?php
        if ( func_num_args() > 1) {
?>
<title><?php echo $title; ?></title>
<?php   }
	if($style!="" )
	{
?>
<link rel="stylesheet" type="text/css" href="css/<?php echo $style; ?>.css"/>
<?php		
	}
	else
	{
?>
<link rel="stylesheet" type="text/css" href="default.css"/>
<?php
	}
?>
</head>
<script src="pc.js"></script>
<body TOPMARGIN="0"
<?php
	if($bkimg)
		echo " background = \"".$bkimg."\" ";
?>
>
<?php
}

function html_format($str,$multi=FALSE)
{
	$str = preg_replace(" ","&nbsp;",htmlspecialchars(stripslashes($str)));	
	if($multi)
		$str = nl2br($str);
	return $str;	
}

function time_format($t)
{
	$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]." ".$t[8].$t[9].":".$t[10].$t[11].":".$t[12].$t[13];
	return $t;
}

function rss_time_format($t)
{
	$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]."T".$t[8].$t[9].":".$t[10].$t[11].":".$t[12].$t[13]."+08:00";
	return $t;
}

function pc_friend_file_open($id,$write="r")
{
	global $pcconfig;
	if(!$id || !stristr("ABCDEFGHIJKLMNOPQRSTUVWXYZ",$id[0]))
	{
		return FALSE;
	}
	else
	{
		$file = $pcconfig["HOME"]."/home/".strtoupper($id[0])."/".$id."/pc_friend";
		if($write=="w")
			$fp = fopen($file,"w");
		elseif($write=="a+")
			$fp = fopen($file,"a+");
		else
		{
			if(file_exists($file))
				$fp = fopen($file,"r");
			else
			{
				return FALSE;
				exit();
			}
		}
		$filesize = filesize($file);
		$file = array(
			"FP" => $fp,
			"SIZE" => $filesize
			);
		return $file;
	}
}

function pc_friend_file_close($fp)
{
	fclose($fp);
}

function pc_is_friend($userid,$uid)
{
	if(!$file = pc_friend_file_open($uid))
		return FALSE;
	else
	{
		$fp = $file["FP"];
		while(!feof($fp))
		{
			$line = trim(fgets($fp,12));	
			if(strtolower($line)==strtolower($userid))
			{
				pc_friend_file_close($fp);
				return $line;
			}	
		}
		pc_friend_file_close($fp);
		return FALSE;
	}
}

function pc_is_admin($currentuser,$pc)
{
	if(strtolower($pc["USER"]) == strtolower($currentuser["userid"]) && $pc["TIME"] > date("YmdHis",$currentuser["firstlogin"]) && $currentuser["firstlogin"])
		return TRUE;
	else
		return FALSE;
}

function pc_friend_list($uid)
{
	$file = pc_friend_file_open($uid,"r");
	if(!$file)
		return NULL;
	$fp = $file["FP"];
	$i = 0;
	while(!feof($fp))
	{
		$line = trim(fgets($fp,12));
		if(!$line)
			continue;
		$friendlist[$i] = $line;
		$i ++ ;
	}
	pc_friend_file_close($fp);
	@sort($friendlist);
	return $friendlist;
}

function pc_add_friend($id,$uid)
{
	if(!$file = pc_friend_file_open($uid,"a+"))
		return FALSE;
	else
	{
		$fp = $file["FP"];
		fputs($fp,$id."\n");
		pc_friend_file_close($fp);
		return TRUE;
	}
}

function pc_del_friend($id,$uid)
{
	$friendlist = pc_friend_list($uid);
	if($file = pc_friend_file_open($uid,"w"))
	{
		$fp = $file["FP"];
		for($i = 0;$i < count($friendlist); $i ++ )
		{
			if(strtolower($id)!=strtolower($friendlist[$i]))
				fputs($fp,$friendlist[$i]."\n");
		}
		pc_friend_file_close($fp);
	}
}

function pc_db_connect()
{
	GLOBAL $db;
	@$link = mysql_connect($db["HOST"],$db["USER"],$db["PASS"]) or die("无法连接到服务器!");
	@mysql_select_db($db["NAME"],$link);
	return $link;
}

function pc_db_close($link)
{
	@mysql_close($link);
}

function pc_init_fav($link,$uid)
{
	$query = "INSERT INTO `nodes` ( `nid` , `pid` , `tid` , `type` , `source` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` ) ".
		" VALUES ('', '0', '0', '1', '', '".$_SERVER["REMOTE_ADDR"]."', '".date("YmdHis")."' , '".date("YmdHis")."', '".$uid."', '0', '0', '', NULL , '3', '0');";
	$r = mysql_query($query,$link);
	return $r;
}

function pc_update_record($link,$uid,$addstr="+0")
{
	$query = "UPDATE users SET `createtime` = `createtime` , `modifytime` = '".date("YmdHis")."' , `nodescount` = `nodescount` ".$addstr." WHERE `uid` = '".$uid."' ";
	mysql_query($query,$link);
}

function pc_visit_counter($link,$uid)
{
	$query = "UPDATE users SET `createtime` = `createtime` , `visitcount` = `visitcount` + 1  WHERE `uid` = '".$uid."' ; ";
	mysql_query($query,$link);
}

function pc_used_space($link,$uid,$access,$pid=0)
{
	if($access == 3)
		$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 0 ; ";
	else
		$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '".$access."'  AND `type` = 0 ; ";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_dir_num($link,$uid,$pid=0)
{
	$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 1 ; ";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_file_num($link,$uid,$pid=0)
{
	$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 0 ; ";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_blog_menu($link,$uid,$tag=9)
{
	if($tag == 9)
		$query = "SELECT * FROM topics WHERE `uid` = '".$uid." ' ORDER BY `sequen` ; ";
	else
		$query = "SELECT * FROM topics WHERE `uid` = '".$uid." ' AND ( `access` = '".$tag."' OR `access` = 9 ) ORDER BY `sequen` DESC ; ";
	$result = mysql_query($query,$link);
	$i = 0;
	while($rows = mysql_fetch_array($result))
	{
		$blog[$i] = array(
				"TID" => $rows[tid],
				"NAME" => $rows[topicname],
				"SEQ" => $rows[sequen],
				"TAG" => $rows[access]
				);
		$i ++;
	}
	mysql_free_result($result);
	$blog[$i] = array(
			"TID" => 0,
			"NAME" => "未归类",
			"SEQ" => 0,
			"TAG" => 9
			);
	
	return $blog;
}

function pc_style_array($i)
{
	switch($i)
	{
		case 1:
			break;
		default:		
			$style = array(
				"TOPIMG" => "style/default/p1.jpg",
				"CSSFILE" => "style/default/default.css"
				);	
	}
	return $style;
}
?>