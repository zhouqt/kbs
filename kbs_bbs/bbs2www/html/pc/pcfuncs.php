<?php

/*
** this file define some functions used in personal corp.
** @id:windinsn Nov 19,2003
*/
require("funcs.php");
require("pcconf.php");//blog配置文件
$db["HOST"]=bbs_sysconf_str("MYSQLHOST");
$db["USER"]=bbs_sysconf_str("MYSQLUSER");
$db["PASS"]=bbs_sysconf_str("MYSQLPASSWORD");
$db["NAME"]=bbs_sysconf_str("MYSQLSMSDATABASE");

$brdarr = array();
$pcconfig["BRDNUM"] = bbs_getboard($pcconfig["BOARD"], $brdarr);

if(!$currentuser["userid"])
		$currentuser["userid"] = "guest";

$pcconfig["NOWRAPSTR"] = "<!--NoWrap-->";
$pcconfig["EDITORALERT"] = "<!--Loading HTMLArea Editor , Please Wait/正在加载 HTML编辑器 ， 请稍候 ……-->";

function pc_html_init($charset,$title="",$otherheader="",$cssfile="",$bkimg="",$loadhtmlarea=FALSE)
{
	global $_COOKIE;
	global $cachemode;
	global $currentuser;
	global $cssFile;
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
	if($cssfile!="" )
	{
?>
<link rel="stylesheet" type="text/css" href="<?php echo $cssfile; ?>"/>
<?php		
	}
	else
	{
?>
<link rel="stylesheet" type="text/css" href="default.css"/>
<?php
	}
	if($loadhtmlarea)
	{
?>
<script type="text/javascript">
_editor_url = "htmlarea/";
</script>
<!-- load the main HTMLArea files -->
<script type="text/javascript" src="htmlarea/htmlarea.js"></script>
<script type="text/javascript" src="htmlarea/lang/en.js"></script>
<script type="text/javascript" src="htmlarea/dialog.js"></script>
<script type="text/javascript" src="htmlarea/popupwin.js"></script>
<style type="text/css">
@import url(htmlarea/htmlarea.css);
textarea { background-color: #fff; border: 1px solid 00f; }
</style>
<!-- load the plugins -->
<script type="text/javascript">
      HTMLArea.loadPlugin("TableOperations");
      HTMLArea.loadPlugin("SpellChecker");
</script>
<script type="text/javascript" defer="1">
var editor = null;
function initEditor() {
  editor = new HTMLArea("blogbody");
  editor.registerPlugin("TableOperations");
  editor.registerPlugin("SpellChecker");
  editor.generate();
  return false;
}
    
</script>
<?php
	}
?>
<script src="pc.js"></script>
</head>
<body TOPMARGIN="0" leftmargin="0"
<?php
	if($loadhtmlarea)
	{
?>
 onload="initEditor()"
<?php
	}
	if($bkimg)
		echo " background = \"".$bkimg."\" ";
?>
>
<?php
}

function undo_html_format($str)
{
	$str = str_replace("&nbsp;"," ",$str);
	$str = str_replace("<br />","\n",$str);
	$str = preg_replace("/&gt;/i", ">", $str);
	$str = preg_replace("/&lt;/i", "<", $str);
	$str = preg_replace("/&quot;/i", "\"", $str);
	$str = preg_replace("/&amp;/i", "&", $str);
	return $str;
}

function html_editorstr_format($str)
{
	global $pcconfig;
	$str = str_replace($pcconfig["EDITORALERT"],"",$str);
	if(strstr($str,$pcconfig["NOWRAPSTR"]))
		$str = $pcconfig["NOWRAPSTR"].str_replace($pcconfig["NOWRAPSTR"],"",$str);
	return $str;
}

function html_format($str,$multi=FALSE,$useHtmlTag = FALSE,$defaultfg = "#000000" , $defaultbg = "#FFFFFF")
{
	global $pcconfig;
	if($multi)
	{
		if(strstr($str,$pcconfig["NOWRAPSTR"]) || $useHtmlTag )
			$str = str_replace("<?","&lt;?",stripslashes($str));
		else
			$str = nl2br(str_replace(" ","&nbsp;",htmlspecialchars(stripslashes($str))));
			//$str = nl2br(ansi_convert(stripslashes($str) , $defaultfg, $defaultbg));	
	}
	else
		$str = str_replace(" ","&nbsp;",htmlspecialchars(stripslashes($str)));	
	return $str;	
}

function time_format($t)
{
	$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]." ".$t[8].$t[9].":".$t[10].$t[11].":".$t[12].$t[13];
	return $t;
}

function time_format_date($t)
{
	$t= "<font class='date'>".$t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]."</font>";
	return $t;
}

function time_format_date1($t)
{
	$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7];
	return $t;
}

function rss_time_format($t)
{
	$t= $t[0].$t[1].$t[2].$t[3]."-".$t[4].$t[5]."-".$t[6].$t[7]."T".$t[8].$t[9].":".$t[10].$t[11].":".$t[12].$t[13]."+08:00";
	return $t;
}

function pc_get_links($linkstr)
{
	if(!$linkstr)
		return NULL;
	$linkarrays = explode("|",$linkstr);	
	$links = array();
	for($i = 0 ; $i < count($linkarrays) ; $i ++ )
	{
		$linkarray = explode("#",$linkarrays[$i]);
		$links[$i] = array("LINK" => base64_decode($linkarray[0]) , "URL" => base64_decode($linkarray[1]) , "IMAGE" => $linkarray[2]?TRUE:FALSE);
	}
	return $links ;
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
	global $pcconfig;
	if( $pc["USER"] == $pcconfig["ADMIN"] && pc_is_manager($currentuser) && intval($_COOKIE["BLOGADMIN"]) )
		return TRUE;
	if( $pc["USER"] != $pcconfig["ADMIN"] && pc_is_manager($currentuser) && intval($_COOKIE["BLOGADMIN"]) )
		return FALSE;
	if(strtolower($pc["USER"]) == strtolower($currentuser["userid"]) && $pc["TIME"] > date("YmdHis",$currentuser["firstlogin"]) && $currentuser["firstlogin"])
		return TRUE;
	else
		return FALSE;
}

function pc_is_manager($currentuser)
{
	global $pcconfig;
	if(!$currentuser || !$currentuser["index"] ) return FALSE;
	$ret = 	bbs_is_bm($pcconfig["BRDNUM"], $currentuser["index"]);
	return $ret ;
}

function pc_in_blacklist($link , $userid , $pcuid = 0)
{
	$query = "SELECT * FROM blacklist WHERE userid = '".addslashes($userid)."' AND ( uid = ".intval($pcuid)." OR uid = 0 );";
	$result = mysql_query( $query , $link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	if($rows)
		return $rows[manager];
	else
		return FALSE;
}

function pc_add_blacklist($link , $userid , $pcuid = 0)
{
	global $currentuser;
	$query = "INSERT INTO `blacklist` ( `userid` , `uid` , `manager` , `hostname` , `addtime` ) ".
		" VALUES ('".addslashes($userid)."', '".intval($pcuid)."', '".addslashes($currentuser["userid"])."', '".addslashes($_SERVER["REMOTE_ADDR"])."', NOW( ));";	
	mysql_query($query,$link);
}

function pc_del_blacklist($link , $userid , $pcuid = 0)
{
	$query = "DELETE FROM blacklist WHERE userid = '".addslashes($userid)."' AND uid = '".intval($pcuid)."';";
	mysql_query($query,$link);
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

function pc_load_infor($link,$userid=FALSE,$uid=0)
{
	global $cssFile;
	if($userid)
		$query = "SELECT * FROM users WHERE `username`= '".$userid."'  LIMIT 0,1;";
	else
		$query = "SELECT * FROM users WHERE `uid` = '".$uid."' LIMIT 0,1;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	if(!$rows)
		return FALSE;
	else
	{
		$pcThem = pc_get_theme($rows[theme]);
		$pc = array(
			"NAME" => html_format($rows[corpusname]),
			"USER" => $rows[username],
			"UID" => $rows[uid],
			"DESC" => html_format($rows[description]),
			"THEM" => $pcThem,
			"TIME" => $rows[createtime],
			"VISIT" => $rows[visitcount],
			"CREATED" => $rows[createtime],
			"MODIFY" => $rows[modifytime],
			"NODES" => $rows[nodescount],
			"NLIM" => $rows[nodelimit],
			"DLIM" => $rows[dirlimit],
			"STYLE" => pc_style_array($rows[style]),
			"LOGO" => str_replace("<","&lt;",stripslashes($rows[logoimage])),
			"BKIMG" => str_replace("<","&lt;",stripslashes($rows[backimage])),
			"LINKS" => pc_get_links(stripslashes($rows[links])),
			"EDITOR" => $rows[htmleditor],
			"INDEX" => array("nodeNum"=> $rows[indexnodes],"nodeChars" => $rows[indexnodechars]),
			"CSSFILE" => htmlspecialchars(stripslashes($rows[cssfile])),
			"EMAIL" => htmlspecialchars(stripslashes($rows[useremail])),
			"FAVMODE" => (int)($rows[favmode]),
			"UPDATE" => (int)($rows[updatetime]),
			"INFOR" => str_replace("<?","&lt;?",stripslashes($rows[userinfor]))
			);
	if($pc["CSSFILE"])
		$cssFile = $pc["CSSFILE"];
	else
		$cssFile = "";
		
	return $pc;
	}
}

function pc_get_theme($theme,$stripSlashes=TRUE)
{
	global $pcconfig;
	if($stripSlashes)
		$theme = stripslashes($theme) ;
	$theme = explode("/",$theme);	
	if(!$pcconfig["SECTION"][$theme[0]]) $theme[0] = "others";
	return $theme;
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
	$query = "UPDATE users SET `createtime` = `createtime` , `visitcount` = `visitcount` + 1  WHERE `uid` = '".$uid."' ;";
	mysql_query($query,$link);
}

function pc_used_space($link,$uid,$access,$pid=0)
{
	if($access == 3)
		$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 0 ;";
	else
		$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '".$access."'  AND `type` = 0 ;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_dir_num($link,$uid,$pid=0)
{
	$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 1 ;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_file_num($link,$uid,$pid=0)
{
	$query = "SELECT COUNT(*) FROM nodes WHERE `uid` = '".$uid."' AND `access` = '3' AND `pid` = '".$pid."' AND `type` = 0 ;";
	$result = mysql_query($query);
	$rows = mysql_fetch_row($result);
	mysql_free_result($result);
	return $rows[0];
}

function pc_blog_menu($link,$uid,$tag=9)
{
	if($tag == 9)
		$query = "SELECT * FROM topics WHERE `uid` = '".$uid." ' ORDER BY `sequen` ;";
	else
		$query = "SELECT * FROM topics WHERE `uid` = '".$uid." ' AND ( `access` = '".$tag."' OR `access` = 9 ) ORDER BY `sequen` DESC ;";
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
			"NAME" => "其他类别",
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
			$style = array(
				"SID" => 1,
				"INDEXFUNC" => "display_blog_smth",
				"TOPIMG" => "style/default/p1.jpg",
				"CSSFILE" => "default.css"
				);
			break;
		case 2:
			$style = array(
				"SID" => 2,
				"INDEXFUNC" => "display_blog_earthsong",
				"TOPIMG" => "style/default/p1.jpg",
				"CSSFILE" => "style/earthsong/earthsong.css"
				);
			break;
		case 9:
			$style = array(
				"SID" => 9,
				);
			break;
		default:		
			$style = array(
				"SID" => 0,
				"INDEXFUNC" => "display_blog_default",
				"TOPIMG" => "style/default/p1.jpg",
				"CSSFILE" => "default.css"
				);	
	}
	return $style;
}

function get_next_month($yy,$mm)
{
	$mm ++;
	if($mm > 12)
	{
		$mm = 1;
		$yy ++ ;
	}	
	return array($yy,$mm);
}

function get_pre_month($yy,$mm)
{
	$mm --;
	if($mm < 1)
	{
		$mm = 12;
		$yy --;
	}
	return array($yy,$mm);
}

function display_blog_catalog()
{
	global $pcconfig;
	$secNum = count($pcconfig["SECTION"]);
	$secKeys = array_keys($pcconfig["SECTION"]);
?>
<center><table cellspacing=0 cellpadding=3 border=0 width=90% class=t1>
<tr>
	<td class=t8><strong>Blog分类&gt;&gt;</strong></td>
</tr>
<tr>
	<td class=t4>
	<table cellspacing=0 cellpadding=3 border=0 width=98% class=f1>
<?php
	for($i = 0 ; $i < $secNum ; $i ++ )
	{
		if( $i % 6 == 0 ) echo "<tr>";		
		echo "<td class=f1 align=center><a href=\"pcsec.php?sec=".htmlspecialchars($secKeys[$i])."\">".htmlspecialchars($pcconfig["SECTION"][$secKeys[$i]])."</a></td>";
		if( $i % 6 == 5 ) echo "</tr>";
	}
?>
	</table>
	</td>
</tr>
</table></center><br />
<?php	
}

function pc_get_user_permission($currentuser,$pc)
{
	global $loginok;
	if(pc_is_admin($currentuser,$pc) && $loginok == 1)
	{
		$sec = array("公开区","好友区","私人区","收藏区","删除区","设定好友","Blog管理","参数设定");
		$pur = 3;
		$tags = array(1,1,1,1,1,1,1,1);
	}
	elseif(pc_is_friend($currentuser["userid"],$pc["USER"]) || pc_is_manager($currentuser))
	{
		$sec = array("公开区","好友区");
		$pur = 1;
		$tags = array(1,1,0,0,0,0,0,0);
		if($pc["FAVMODE"] == 1 || $pc["FAVMODE"] == 2)//收藏夹模式
		{
			$sec[3] = "收藏区";
			$tags[3] = 1;
		}
	}
	else
	{
		$sec = array("公开区");
		$pur = 0;
		$tags = array(1,0,0,0,0,0,0,0);
		if($pc["FAVMODE"] == 2)//收藏夹模式
		{
			$sec[3] = "收藏区";
			$tags[3] = 1;
		}
	}	
	return array(
		"tags" => $tags ,
		"pur" => $pur , 
		"sec" => $sec  
		);
}

function pc_select_blogtheme($theme,$themeValue="pcthem")
{
	global $pcconfig;
?>
<select name="<?php echo $themeValue; ?>" class="f1">
<?php
	reset($pcconfig["SECTION"]);
	while( list($sec , $secName) = each($pcconfig["SECTION"]) )
	{
		if($theme[0] == $sec)
			echo "<option value=\"".htmlspecialchars($sec)."\" selected>".htmlspecialchars($secName)."</option>";
		else
			echo "<option value=\"".htmlspecialchars($sec)."\">".htmlspecialchars($secName)."</option>";
	}	
?>
</select>
<?php
}

function pc_cache( $modifytime )
{
	$lastmodifytime = time_format( $modifytime );
	$lastmodifytime = strtotime( $lastmodifytime );
	if (cache_header("public",$lastmodifytime,300))
		return TRUE;
	else
		return FALSE;
}

function pc_main_navigation_bar()
{
	global $pcconfig;
?>
<p align="center">
[<a href="pcmain.php">Blog首页</a>]
[<a href="pc.php">用户列表</a>]
[<a href="pcsec.php">分类目录</a>]
[<a href="pcreclist.php">推荐文章</a>]
[<a href="pcnew.php">最新文章</a>]
[<a href="pcnew.php?t=c">最新评论</a>]
[<a href="pcsearch2.php">Blog搜索</a>]
[<a href="pcnsearch.php">文章搜索</a>]
[<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">Blog论坛</a>]
[<a href="pcapp0.html">Blog申请</a>]
<?php
	if( $pcconfig["ADMIN"] ){
?>
[<a href="index.php?id=<?php echo $pcconfig["ADMIN"]; ?>">帮助主题</a>]
<?php
	}
?>
</p><p align="center">
[<b>RSS频道</b>&nbsp;
最新日志<a href="rssnew.php"><img src="images/xml.gif" border="0" align="absmiddle" alt="本站最新Blog日志"></a>
推荐文章<a href="rssrec.php"><img src="images/xml.gif" border="0" align="absmiddle" alt="本站推荐Blog日志"></a>
]
</p>
<?php	
}

function pc_update_cache_header($updatetime = 10)
{
	global $cachemode;
	$scope = "public";
	$modifytime=time();
	$expiretime=300;
	session_cache_limiter($scope);
	$cachemode=$scope;
	@$oldmodified=$_SERVER["HTTP_IF_MODIFIED_SINCE"];
	if ($oldmodified!="") {
                $oldtime=strtotime($oldmodified);
	} else $oldtime=0;
	if ($modifytime - $oldtime < 60 * $updatetime ) {
		header("HTTP/1.1 304 Not Modified");
	        header("Cache-Control: max-age=" . "$expiretime");
		return TRUE;
	}
	header("Last-Modified: " . gmdate("D, d M Y H:i:s", $modifytime) . "GMT");
	header("Expires: " . gmdate("D, d M Y H:i:s", $modifytime+$expiretime) . "GMT");
	header("Cache-Control: max-age=" . "$expiretime");
	return FALSE;
}

function pc_logs($link , $action , $comment = "" , $pri_id = "" , $sec_id = "")
{
	global $currentuser;
	if( !$action ) 
		return FALSE;
	
	$query = "INSERT INTO `logs` ( `lid` , `username` , `hostname` , `ACTION` , `pri_id` , `sec_id` , `COMMENT` , `logtime` )".
		"VALUES ('', '".addslashes($currentuser[userid])."', '".addslashes($_SERVER["REMOTE_ADDR"])."', '".addslashes($action)."', '".addslashes($pri_id)."', '".addslashes($sec_id)."', '".addslashes($comment)."', NOW( ) );";
	mysql_query($query,$link);
	return TRUE;
}

function pc_counter($link)
{
	global $pc,$currentuser;
	if(!$pc || !is_array($pc))
		return FALSE;
	$visitcount = $_COOKIE["BLOGVISITCOUNT"];
	$action = $currentuser["userid"]." visit ".$pc["USER"]."'s Blog(www)";
	if(!$visitcount)
	{
		$query = "SELECT logtime FROM logs WHERE hostname = '".addslashes($_SERVER["REMOTE_ADDR"])."' AND username = '".addslashes($currentuser[userid])."' AND pri_id = '".addslashes($pc["USER"])."' ORDER BY lid DESC LIMIT 0,1;";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		mysql_free_result($result);
		if( !$rows )
		{
			pc_visit_counter($link,$pc["UID"]);//计数器加1
			pc_logs($link,$action,"",$pc["USER"]);//记一下访问日志
			$pc["VISIT"] ++;
			$visitcount = ",".$pc["UID"].",";
			setcookie("BLOGVISITCOUNT",$visitcount);
			return;
		}
		elseif( date("YmdHis") - $rows[logtime] > 10000 )//1个小时log一次 
		{
			pc_visit_counter($link,$pc["UID"]);//计数器加1
			pc_logs($link,$action,"",$pc["USER"]);//记一下访问日志
			$pc["VISIT"] ++;
			$visitcount = ",".$pc["UID"].",";
			setcookie("BLOGVISITCOUNT",$visitcount);
			return;
		}
		else
			return;
	}
	elseif(!stristr($visitcount,",".$pc["UID"].","))
	{
		pc_visit_counter($link,$pc["UID"]);//计数器加1
		pc_logs($link,$action,"",$pc["USER"]);//记一下访问日志
		$pc["VISIT"] ++;
		$visitcount .= $pc["UID"].",";
		setcookie("BLOGVISITCOUNT",$visitcount);
		return;
	}	
}

function pc_node_counter($link,$nid)
{
	$query = "UPDATE nodes SET visitcount = visitcount + 1 , changed  = changed  WHERE `nid` = '".$nid."' ;";
	mysql_query($query,$link);
}

function pc_ncounter($link,$nid)
{
	if(!$_COOKIE["BLOGREADNODES"])
	{
		$readnodes = ",".$nid.",";
		setcookie("BLOGREADNODES",$readnodes);
		pc_node_counter($link,$nid);
	}
	elseif(!stristr($_COOKIE["BLOGREADNODES"],",".$nid.","))
	{
		$readnodes = $_COOKIE["BLOGREADNODES"] . $nid.",";
		setcookie("BLOGREADNODES",$readnodes);
		pc_node_counter($link,$nid);
	}
}

/*
**	0: XSL 
**	1: CSS
*/
function pc_load_stylesheet($link,$pc)
{
	$query = "SELECT stylesheet FROM userstyle WHERE uid = '".$pc["UID"]."' LIMIT 0 , 1;";	
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
		return FALSE;
	return intval($rows[stylesheet]);
}
?>
