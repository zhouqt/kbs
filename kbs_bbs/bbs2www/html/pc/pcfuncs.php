<?php

/*
** this file define some functions used in personal corp.
** @id:windinsn Nov 19,2003
*/
require("funcs.php");
require("pcconf.php");//blog配置文件
require("pctbp.php");//引用通告相关函数
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

function pc_html_init($charset,$title="",$otherheader="",$cssfile="",$bkimg="",$htmlEditor=0)
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
	if($htmlEditor==1)//use htmlarea editor
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
	if($htmlEditor==1)
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
			$str = str_replace("<?","&lt;?",$str);
		else
			$str = nl2br(str_replace(" ","&nbsp;",htmlspecialchars($str)));
			//$str = nl2br(ansi_convert(stripslashes($str) , $defaultfg, $defaultbg));	
	}
	else
		$str = str_replace(" ","&nbsp;",htmlspecialchars($str));	
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

function pc_is_member($pc,$userid)
{
	global $currentuser;
	if(pc_is_manager($currentuser))
		return TRUE;
	
	if(!$pc || !is_array($pc))
		return FALSE;
	
	$query = "SELECT uid FROM members WHERE uid = '".intval($pc["UID"])." AND username = '".addslashes($userid)."' LIMIT 0 , 1;";
	$result = mysql_query($query);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	if(!$rows)
		return FALSE;
	else
		return TRUE;
}

function pc_get_members($link,$pc)
{
	if(!$pc || !is_array($pc))
		return FALSE;
	if($pc["TYPE"]!=1)
		return FALSE;
	$members = array();
	$query = "SELECT username FROM members WHERE uid = '".intval($pc["UID"])."';";	
	$result = mysql_query($query,$link);
	while($rows = mysql_fetch_array($result))
		$members[] = $rows[username];
	mysql_free_result($result);
	return $members;
}

function pc_add_member($link,$pc,$userid)
{
	global $currentuser;
	if(!$pc || !is_array($pc))
		return FALSE;
	if($pc["TYPE"]!=1)
		return FALSE;
	$lookupuser = array();
	if(bbs_getuser($userid,$lookupuser)==0)
		return FALSE;
	
	$userid = $lookupuser["userid"];
	$query = "INSERT INTO `members` ( `uid` , `username` ) ".
	         "VALUES ( '".intval($pc["UID"])."', '".addslashes($userid)."' );";
	if(!mysql_query($query,$link))
		return FALSE;
	
	$action = "ADD MEMBER: ".$userid; 
	if(!pc_group_logs($link,$pc,$action))
		exit("公有BLOG LOG错误");
	return TRUE;
}

function pc_del_member($link,$pc,$userid)
{
	if(!$pc || !is_array($pc))
		return FALSE;
	if($pc["TYPE"]!=1)
		return FALSE;
	$query = "DELETE FROM members WHERE uid = '".$pc["UID"]."' AND username = '".addslashes($userid)."' LIMIT 1;";
	if(!mysql_query($query,$link))
		return FALSE;
	
	$action = "DEL MEMBER: ".$userid; 
	if(!pc_group_logs($link,$pc,$action))
		exit("公有BLOG LOG错误");
	return TRUE;
}

function pc_is_admin($currentuser,$pc)
{
	global $pcconfig;
	if( $pc["TYPE"] == 1 )
	{
		return pc_is_member($currentuser,$pc);
	}
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
		$line = trim(fgets($fp,14));
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
		fputs($fp,$id."\n",strlen($id)+2);
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
				fputs($fp,$friendlist[$i]."\n",strlen($friendlist[$i])+2);
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
		$query = "SELECT * FROM users WHERE `username`= '".addslashes($userid)."'  LIMIT 0,1;";
	else
		$query = "SELECT * FROM users WHERE `uid` = '".intval($uid)."' LIMIT 0,1;";
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
			"INFOR" => str_replace("<?","&lt;?",stripslashes($rows[userinfor])),
			"TYPE" => $rows[pctype],
			"LOGTID" => $rows[logtid] 
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
[<a href="pcmain.php">首页</a>]
[<a href="pc.php">用户</a>]
[<a href="pclist.php">热门排行</a>]
[<a href="pcsec.php">分类</a>]
[<a href="pcreclist.php">推荐文章</a>]
[<a href="pcnew.php">最新日志</a>]
[<a href="pcnew.php?t=c">最新评论</a>]
[<a href="pcsearch2.php">博客搜索</a>]
[<a href="pcnsearch.php">日志搜索</a>]
[<a href="/bbsdoc.php?board=<?php echo $pcconfig["BOARD"]; ?>">论坛</a>]
[<a href="pcapp0.html"><font color=red>申请</font></a>]
<?php
	if( $pcconfig["ADMIN"] ){
?>
[<a href="index.php?id=<?php echo $pcconfig["ADMIN"]; ?>">帮助主题</a>]
<?php
	}
?>
</p><p align="center">
[<b><img src="images/xml.gif" border="0" align="absmiddle" alt="XML">RSS频道</b>&nbsp;
<a href="rssnew.php">最新日志</a>
<a href="rssrec.php">推荐文章</a>
<a href="opml.php?t=2">新用户(OPML)</a>
<a href="opml.php">最近更新(OPML)</a>
<a href="opml.php?t=1">访问最多(OPML)</a>
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

function html_format_fix_length($str,$length)
{
	if(strlen($str) <= $length )
		return $str;
	$str = substr($str,0,$length);
	$str .= "...";
	return $str;	
}

function myAddslashes($str)
{
    $str = addslashes($str);
    $str = str_replace("_","\_",$str);
    $str = str_replace("%","\%",$str);
    return $str;
}

function pc_load_topic($link,$uid,$tid,&$topicname,$access=9)
{
	$uid = intval($uid);
	$tid = intval($tid);
	$access = intval($access);
	
	if($access == 9)
		$query = "SELECT topicname FROM topics WHERE tid = ".$tid." AND uid = ".$uid." LIMIT 0 , 1;";
	else
		$query = "SELECT topicname FROM topics WHERE tid = ".$tid." AND uid = ".$uid." AND access = ".$access." LIMIT 0 , 1;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
		return FALSE;
	
	mysql_free_result($result);
	$topicname = $rows[topicname];
	return $tid;
}

function pc_load_directory($link,$uid,$pid)
{
	$uid = intval($uid);
	$pid = intval($pid);
	
	$query = "SELECT `nid` FROM nodes WHERE `uid` = '".$uid."' AND `access` = 3 AND `nid` = '".$pid."' AND `type` = 1 LIMIT 1;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
		return FALSE;
	
	mysql_free_result($result);
	return TRUE;
}

/*
** add a node
** $pc  : pc infor-->load by pc_load_infor() function
** return  0  :seccess
**         -1 :缺少主题
**         -2 :收藏夹目录不存在
**         -3 :目标文件夹超过文章上限
**         -4 :目标分类不存在
**         -5 :数据库添加错误
**         -6 :系统错误导致引用通告发送失败
**         -7 :引用通告的url错误
**         -8 :引用通告目标服务器连接超时
*/
function pc_add_node($link,$pc,$pid,$tid,$emote,$comment,$access,$htmlTag,$trackback,$subject,$body,$nodeType,$tbpUrl="",$tbpArt="")
{
	global $pcconfig;
	
	$pid = intval($pid);
	$tid = intval($tid);
	$emote = intval($emote);
	$comment = ($comment==1)?1:0;
	$access = intval($access);
	$htmlTag = ($htmlTag==1)?1:0;
	$trackback = ($trackback==1)?1:0;
	$subject = addslashes($subject);
	$body = html_editorstr_format($body);
	$nodeType = intval($nodeType); //0: 普通;1: log,不可删除
	
	if(!$pc || !is_array($pc))
		return FALSE;
	
	if(!$subject) //检查主题
		return -1;
	
	if($access < 0 || $access > 4 )
		$access = 2;//如果参数错误先在私人区发表
	
	if($access == 3) //若是发表在收藏区，检查目标文件夹
	{
		if(!pc_load_directory($link,$pc["UID"],$pid))
			return -2;
		if(pc_used_space($link,$pc["UID"],3,$pid) >= $pc["NLIM"]) //目标文件夹使用空间
			return -3;
		$tid = 0;
	}
	else
	{
		$pid = 0;
		if(pc_used_space($link,$pc["UID"],$tag) >= $pc["NLIM"]) //目标文件夹使用空间
			return -3;
		if($tid != 0) //如果是发布在一个分类下面，需要检查分类
		{
			if(!pc_load_topic($link,$pc["UID"],$tid,&$topicname,$access))
				return -4;
		}
	}
	
	if($access != 0) //公开区以外不发布引用通告
		$tbpUrl = "";
	
	if($tbpUrl && pc_tbp_check_url($tbpUrl) && $tbpArt) //若有引用通告的相关文章，加上链接
	{
		if($htmlTag)
			$body .= "<br /><br /><strong>相关文章</strong><br />\n".
			         "<a href='".$tbpArt."'>".$tbpArt."</a>";
		else
			$body .= "\n\n[相关文章]\n".$tbpArt;
		
	}
	$body = addslashes($body);
	
	//日志入库
	$query = "INSERT INTO `nodes` (  `pid` , `tid` , `type` , `source` , `emote` , `hostname` , `changed` , `created` , `uid` , `comment` , `commentcount` , `subject` , `body` , `access` , `visitcount` , `htmltag`,`trackback` ,`trackbackcount`,`nodetype`) ".
	   	 "VALUES ( '".$pid."', '".$tid."' , '0', '', '".$emote."' ,  '".addslashes($_SERVER["REMOTE_ADDR"])."',NOW( ) , NOW( ), '".$pc["UID"]."', '".$comment."', '0', '".$subject."', '".$body."', '".$access."', '0' , '".$htmlTag."' ,'".$trackback."','0','".$nodeType."');";
	if(!mysql_query($query,$link))
		return -5;
	
	//公开区文章发布后更新文章数
	if($access == 0)
		pc_update_record($link,$pc["UID"],"+1");
	
	if($tbpUrl) //发送引用通告
	{
		//提取日志的nid
		$query = "SELECT `nid` FROM nodes WHERE `subject` = '".$subject."' AND `body` = '".$body."' AND `uid` = '".$pc["UID"]."' AND `access` = '".$access."' AND `pid` = '".$pid."' AND `tid` = '".$tid."' ORDER BY nid DESC LIMIT 0,1;";
		$result = mysql_query($query,$link);
		$rows = mysql_fetch_array($result);
		
		if(!$rows)
			return -6;
		
		$thisNid = $rows[nid];
		mysql_free_result($result);
		
		if($htmlTag)
			$tbbody = strip_tags($body);
		else
			$tbbody = $body;
		
		if(strlen($tbbody) > 255 )
			$tbbody = substr($tbbody,0,251)." ...";
		
		$tbarr = array(
				"title" => stripslashes($subject),
				"excerpt" => stripslashes($tbbody),
				"url" => "http://".$pcconfig["SITE"]."/pc/pccon.php?id=".$pc["UID"]."&tid=".$tid."&nid=".$thisNid."&s=all",
				"blogname" => undo_html_format($pc["NAME"])
				);	
		
		$r = pc_tbp_trackback_ping($tbpUrl,$tbarr);
		if($r == -1)
			return -7;
		if($r == -2)
			return -8;
	}
	
	return 0;
}

//获取收藏夹根目录的pid
function pc_fav_rootpid($link,$uid)
{
	$query = "SELECT `nid` FROM nodes WHERE `access` = '3' AND  `uid` = '".intval($uid)."' AND `pid` = '0' AND `type` = '1' LIMIT 0 , 1 ;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	if(!$rows)
		return FALSE;
	mysql_free_result($result);	
	return $rows[nid];
}

//公有BLOG的log
function pc_group_logs($link,$pc,$action,$content="")
{
	global $currentuser;
	if($pc["TYPE"] != 1 || !$pc["LOGTID"])
		return FALSE;
	if(!$action)
		return FALSE;
	
	$action = "[".date("Y-m-d H:i:s")."@".$_SERVER["REMOTE_ADDR"]."]".$currentuser["userid"]." ".$action;	 
	$ret = pc_add_node($link,$pc,0,$pc["LOGTID"],0,1,2,0,0,$action,$content,1);
	if($ret != 0)
		return FALSE;
	return TRUE;
}

//分区下添加分类
function pc_add_topic($link,$pc,$access,$topicname)
{
	if(!$pc || !is_array($pc))
		return FALSE;
	if(!$topicname)
		return FALSE;
	$access = intval($access);
	if($access < 0 || $access > 2)
		$access = 2;
	
	$query = "INSERT INTO `topics` (`uid` , `access` , `topicname` , `sequen` ) ".
		"VALUES ( '".$pc["UID"]."', '".$access."', '".addslashes($topicname)."', '0');";
	if(!mysql_query($query,$link))
		return FALSE;
	return TRUE;
}

/*
** 把一个BLOG变成公有BLOG
** return 0 : seccess
**        -1:$pc参数错误
**        -2:已是公有BLOG
**        -3:LOG目录初始化错误
**        -4:系统错误
**        -5:LOG 错误
*/
function pc_convertto_group($link,$pc)
{
	if(!$pc || !is_array($pc))
		return -1;
	if($pc["TYPE"] == 1)
		return -2;
	
	if(!pc_add_topic($link,$pc,2,"GROUPWORK LOGs"))
		return -3;
	
	$query = "SELECT tid FROM topics WHERE uid = '".$pc["UID"]."' AND access = 2 AND topicname = 'GROUPWORK LOGs' ORDER BY tid DESC LIMIT 0,1;";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
	if(!$rows)
		return -3;
	$logtid = $rows[tid];
	
	$query = "UPDATE users SET createtime = createtime , pctype = 1 , logtid = ".$logtid." WHERE uid = ".$pc["UID"]." LIMIT 1;";
	if(!mysql_query($query,$link))
		return -4;
	
	$pc["TYPE"] = 1;
	$pc["LOGTID"] = $logtid;
	
	if(!pc_group_logs($link,$pc,"CONVERT TO GROUPWORK"))
		return -5;
	return 0;
}

function pc_ubb_parse($txt)
{
	$bbcode_lib = $_SERVER["DOCUMENT_ROOT"]."/pc/bbcode.php";
	if(file_exists($bbcode_lib))
		include("bbcode.php");
	else
		return $txt;
	
	$html_entities_match = array('#&(?!(\#[0-9]+;))#', '#<#', '#>#');
	$html_entities_replace = array('&amp;', '&lt;', '&gt;');
	$unhtml_specialchars_match = array('#&gt;#', '#&lt;#', '#&quot;#', '#&amp;#');
	$unhtml_specialchars_replace = array('>', '<', '"', '&');
	
	$bbcode_uid = make_bbcode_uid();
	
	$txt = htmlspecialchars(trim(stripslashes($txt)));
	$txt = trim(addslashes(preg_replace($unhtml_specialchars_match, $unhtml_specialchars_replace, $txt)));
	$txt = preg_replace($html_entities_match, $html_entities_replace, $txt);
	$txt = bbencode_first_pass($txt, $bbcode_uid);
	$txt = stripslashes($txt);
	$txt = bbencode_second_pass($txt, $bbcode_uid);
	$txt = make_clickable($txt);
	$txt = str_replace("\n", '<br />', $txt);
	
	return $txt;
}

function pc_ubb_content($txt="")
{
?>
<SPAN class=gen><SPAN class=genmed></SPAN>
            <TABLE cellSpacing=0 cellPadding=2 border=0>
              <TBODY>
              <TR vAlign=center align=left>
                <TD width="35"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('b')" style="FONT-WEIGHT: bold; WIDTH: 30px" accessKey=b onclick=bbstyle(0) type=button value=" B " name=addbbcode0> 
                  </SPAN></TD>
                <TD width="35"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('i')" style="WIDTH: 30px; FONT-STYLE: italic" accessKey=i onclick=bbstyle(2) type=button value=" i " name=addbbcode2> 
                  </SPAN></TD>
                <TD width="35"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('u')" style="WIDTH: 30px; TEXT-DECORATION: underline" accessKey=u onclick=bbstyle(4) type=button value=" u " name=addbbcode4> 
                  </SPAN></TD>
                <TD width="55"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('q')" style="WIDTH: 50px" accessKey=q onclick=bbstyle(6) type=button value=Quote name=addbbcode6> 
                  </SPAN></TD>
                <TD width="45"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('c')" style="WIDTH: 40px" accessKey=c onclick=bbstyle(8) type=button value=Code name=addbbcode8> 
                  </SPAN></TD>
                <TD width="45"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('l')" style="WIDTH: 40px" accessKey=l onclick=bbstyle(10) type=button value=List name=addbbcode10> 
                  </SPAN></TD>
                <TD width="45"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('o')" style="WIDTH: 40px" accessKey=o onclick=bbstyle(12) type=button value=List= name=addbbcode12> 
                  </SPAN></TD>
                <TD width="45"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('p')" style="WIDTH: 40px" accessKey=p onclick=bbstyle(14) type=button value=Img name=addbbcode14> 
                  </SPAN></TD>
                <TD width="45"><SPAN class=genmed><INPUT class=ubbbutton onmouseover="helpline('w')" style="WIDTH: 40px; TEXT-DECORATION: underline" accessKey=w onclick=bbstyle(16) type=button value=URL name=addbbcode16> 
                  </SPAN></TD>
                <TD width="500"><SPAN class=genmed> </TD>
                   </TR>
              <TR>
                <TD colSpan=10>
                  <TABLE cellSpacing=0 cellPadding=0 width="100%" border=0>
                    <TBODY>
                    <TR>
                      <TD><SPAN class=genmed>&nbsp;字体颜色: <SELECT 
                        onmouseover="helpline('s')" 
                        onchange="bbfontstyle('[color=' + this.form.addbbcode18.options[this.form.addbbcode18.selectedIndex].value + ']', '[/color]');this.selectedIndex=0;" 
                        name=addbbcode18> <OPTION class=genmed 
                          style="COLOR: black; BACKGROUND-COLOR: #fafafa" 
                          value=#444444 selected>标准</OPTION> <OPTION 
                          class=genmed 
                          style="COLOR: darkred; BACKGROUND-COLOR: #fafafa" 
                          value=darkred>深红</OPTION> <OPTION class=genmed 
                          style="COLOR: red; BACKGROUND-COLOR: #fafafa" 
                          value=red>红色</OPTION> <OPTION class=genmed 
                          style="COLOR: orange; BACKGROUND-COLOR: #fafafa" 
                          value=orange>橙色</OPTION> <OPTION class=genmed 
                          style="COLOR: brown; BACKGROUND-COLOR: #fafafa" 
                          value=brown>棕色</OPTION> <OPTION class=genmed 
                          style="COLOR: yellow; BACKGROUND-COLOR: #fafafa" 
                          value=yellow>黄色</OPTION> <OPTION class=genmed 
                          style="COLOR: green; BACKGROUND-COLOR: #fafafa" 
                          value=green>绿色</OPTION> <OPTION class=genmed 
                          style="COLOR: olive; BACKGROUND-COLOR: #fafafa" 
                          value=olive>橄榄</OPTION> <OPTION class=genmed 
                          style="COLOR: cyan; BACKGROUND-COLOR: #fafafa" 
                          value=cyan>青色</OPTION> <OPTION class=genmed 
                          style="COLOR: blue; BACKGROUND-COLOR: #fafafa" 
                          value=blue>蓝色</OPTION> <OPTION class=genmed 
                          style="COLOR: darkblue; BACKGROUND-COLOR: #fafafa" 
                          value=darkblue>深蓝</OPTION> <OPTION class=genmed 
                          style="COLOR: indigo; BACKGROUND-COLOR: #fafafa" 
                          value=indigo>靛蓝</OPTION> <OPTION class=genmed 
                          style="COLOR: violet; BACKGROUND-COLOR: #fafafa" 
                          value=violet>紫色</OPTION> <OPTION class=genmed 
                          style="COLOR: white; BACKGROUND-COLOR: #fafafa" 
                          value=white>白色</OPTION> <OPTION class=genmed 
                          style="COLOR: black; BACKGROUND-COLOR: #fafafa" 
                          value=black>黑色</OPTION></SELECT> &nbsp;字体大小:<SELECT 
                        onmouseover="helpline('f')" 
                        onchange="bbfontstyle('[size=' + this.form.addbbcode20.options[this.form.addbbcode20.selectedIndex].value + ']', '[/size]')" 
                        name=addbbcode20> <OPTION class=genmed 
                          value=7>最小</OPTION> <OPTION class=genmed 
                          value=9>小</OPTION> <OPTION class=genmed value=12 
                          selected>正常</OPTION> <OPTION class=genmed 
                          value=18>大</OPTION> <OPTION class=genmed 
                          value=24>最大</OPTION></SELECT> </SPAN>
                          <SPAN class=gensmall><A 
                        class=genmed onmouseover="helpline('a')" 
                        href="javascript:bbstyle(-1)">完成标签</A></SPAN>
                          </TD>
                      <TD noWrap align=left> </TD></TR></TBODY></TABLE></TD></TR>
              <TR>
                <TD colSpan=10><SPAN class=gensmall><INPUT class=helpline 
                  style="FONT-SIZE: 12px; WIDTH: 450px" maxLength=100 size=45 
                  value="提示: 文字风格可以快速使用在选择的文字上" name=helpbox> </SPAN></TD></TR>
              <TR>
                <TD colSpan=10><SPAN class=gen>
                <TEXTAREA onkeyup=storeCaret(this); onclick=storeCaret(this); tabIndex=3 onselect=storeCaret(this); name="blogbody" style="font-size: 14px ; line-height:20px;" cols="100" rows="20" id="blogbody"  onkeydown='if(event.keyCode==87 && event.ctrlKey) {document.postform.submit(); return false;}'  onkeypress='if(event.keyCode==10) return document.postform.submit()' wrap="physical"><?php echo $txt; ?></TEXTAREA> 
                </SPAN></TD></TR></TBODY></TABLE></SPAN>
<?php
}
?>