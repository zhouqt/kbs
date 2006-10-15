<?php
/*
**  为水木清华blog申请定制的申请处理程序
**  处理后完成添加blog用户，$pcconfig["APPBOARD"]版面通告，Email通知
**  不通过申请则发Email通知用户
**  @windinsn Mar 28 , 2004
*/
require("pcadmin_inc.php");
pc_admin_check_permission();
$link = pc_db_connect();

/*
** 载入新申请资料
** type = 0 : 已通过的申请
**        1 : 待处理
**        2 : 驳回的申请
**        3 : 以后不得申请
**
*/
function pc_apply_users($link,$type,$start,$pagesize,$userid="",$appname="")
{
	$type = intval($type);
	$order = ($type == 1)?"ASC":"DESC";
	if( $userid || $appname )
		$query = "SELECT * FROM newapply WHERE ( username = '".addslashes($userid)."' OR appname = '".addslashes($appname)."' ) AND management = ".$type." ORDER BY naid ".$order." LIMIT ".$start." , ".$pagesize." ;";
	else
		$query = "SELECT * FROM newapply WHERE management = ".$type." ORDER BY naid ".$order." LIMIT ".$start." , ".$pagesize." ;";
	$result = mysql_query($query,$link);
	$newApp = array();
	while($rows = mysql_fetch_array($result))
		$newApp[] = $rows;
	return $newApp;
}

function pc_add_users($link,$userid,$corpusname,$manual,$blogtype="",$groupmanager="")
{
	global $pcconfig , $currentuser , $bbsman_modes;
	if(!$userid || !$corpusname)
		return -1;
	
	if (!$blogtype)
	    $blogtype = "normal";
	    
	if ($blogtype=="normal") {
    	$lookupuser=array ();
    	if(bbs_getuser($userid, $lookupuser) == 0 )
    		return -2;
	    $userid = $lookupuser["userid"];
	}
	else {
	    if (!$pcconfig["TYPES"][$blogtype])
	        return -3;
	    $userid = $pcconfig["TYPES"][$blogtype] . '.' . $userid;
	    if (!$groupmanager)
	        return -4;
	    $lookupuser=array ();
    	if(bbs_getuser($groupmanager, $lookupuser) == 0 )
    		return -4;
	    $groupmanager = $lookupuser["userid"];
	}
	
	//更新申请表
	if($manual)
		$query = "INSERT INTO `newapply` ( `naid` , `username` , `appname` , `appself` , `appdirect` , `hostname` , `apptime` , `manager` , `management` ) ".
	 		 "VALUES ('', '".addslashes($userid)."', '".addslashes($corpusname)."', '', '', '".addslashes($_SERVER["REMOTE_ADDR"])."', NOW( ) , '".addslashes($currentuser["userid"])."' , '0');";
	else
		$query = "UPDATE newapply SET apptime = apptime ,manager = '".addslashes($currentuser["userid"])."',management = '0' WHERE username = '".addslashes($userid)."'";// ORDER BY naid DESC LIMIT 1 ;";
	if(!mysql_query($query,$link))
	{
		$errstr = "MySQL Error: ".mysql_error($link);
		pc_db_close($link);
		exit($errstr);
	}
	
	if(pc_load_infor($link,$userid))
		return -5;
	
	if($manual)
	{
		$query = "SELECT username FROM newapply WHERE management != 1 AND management != 3  AND management != 0 AND username = '".addslashes($userid)."' LIMIT 0 , 1;";	
		$result = mysql_query($query,$link);
		if($rows = mysql_fetch_array($result))
			return -6;
	}
	
	//分配个人空间
	if ($pcconfig["USERFILES"])
	{
	    $userfile_limit = $pcconfig["USERFILESLIMIT"];
	    $userfile_num_limit = $pcconfig["USERFILESNUMLIMIT"];
	}
	else
	    $userfile_limit = $userfile_num_limit = 0;
	
	//添加用户
	$query = "INSERT INTO `users` ( `uid` , `username` , `corpusname` , `description` , `theme` , `nodelimit` , `dirlimit` , `createtime` , `style` , `backimage` , `visitcount` , `nodescount` , `logoimage` , `modifytime` , `links` , `htmleditor` , `indexnodechars` , `indexnodes` , `useremail` , `favmode` , `updatetime` , `userinfor` , `pctype` ,`defaulttopic`,`userfile`,`filelimit`) ".
		 "VALUES ('', '".addslashes($userid)."', '".addslashes($corpusname)."', '".addslashes($corpusname)."' , 'others', '300', '300', NOW( ) , '0', '' , '0', '0', '' , NOW( ) , '', '1', '600', '5', '', '0', NOW( ) , '' , '0' , '其他类别' , '".$userfile_limit."','".$userfile_num_limit."');";
	if(!mysql_query($query,$link))
	{
		$errstr = "MySQL Error: ".mysql_error($link);
		pc_db_close($link);
		exit($errstr);
	}
	
	//log一下
	if ($blogtype!="normal") {
	    $action = $groupmanager. " 申请建立群体Blog：" . $userid . "(www)";
	    pc_logs($link , $action , "" , $userid );
	}
	$action = $currentuser["userid"]. " 通过 " . $userid . " 的BLOG申请(www)";
	pc_logs($link , $action , "" , $userid );
		
	//发布公告
	if ($blogtype=="normal") {
		bbs_user_setflag($userid, BBS_PCORP_FLAG, 1);
    	$annTitle = "[公告] 批准 ".$userid." 的 Blog 申请";
    	$annBody =  "\n\n        根据用户 ".$userid." 申请，经审核、讨论后决定开通该用户\n".
    		    "    Blog ，Blog 名称“".$corpusname."”。\n\n".
    		    "        Blog 大部分功能提供在web 模式下，Blog 名称、描述、\n".
    		    "    分类等属性请用户在web 登录后自行修改。\n\n";
	}
	else {
	    $annTitle = "[公告] 开设 ".$userid." 群体Blog";
	    $annBody =  "\n\n        根据用户 ".$groupmanager." 申请，经审核、讨论后决定开设\n".
	        "    ".$userid." 群体Blog，Blog 名称“".$corpusname."”。\n\n".
		    "        Blog 大部分功能提供在web 模式下，Blog 名称、描述、\n".
		    "    分类等属性请群体Blog管理员在web 登录后自行修改。\n\n";
	}
	
	$ret = bbs_postarticle($pcconfig["APPBOARD"], preg_replace("/\\\(['|\"|\\\])/","$1",$annTitle), preg_replace("/\\\(['|\"|\\\])/","$1",$annBody), 0 , 0 , 0 , 0);
	if($ret != 0)
		return -7;
	//发信件给用户
  	$ret = bbs_postmail(($blogtype=="normal")?$userid:$groupmanager,preg_replace("/\\\(['|\"|\\\])/","$1",$annTitle), preg_replace("/\\\(['|\"|\\\])/","$1",$annBody),0,0);
    if($ret < 0)
	    return -8;
	//标记公告
	//bbs_bmmanage($pcconfig["APPBOARD"],$ret,$bbsman_modes["MARK"],0);
	if($manual && $blogtype!="normal") {
	    $pcc = pc_load_infor($link,$userid);
	    if (!$pcc) return -9;
	    pc_convertto_group($link,$pcc);
	    pc_add_member($link,$pcc,$groupmanager);
	}
	return 0;
}

function pc_reject_apply($link,$userid,$applyAgain)
{
	global $currentuser;
	if(!$userid) return FALSE;
	if($applyAgain != 3) $applyAgain = 2;
	$query = "UPDATE newapply SET apptime = apptime ,manager = '".addslashes($currentuser["userid"])."',management = '".$applyAgain."' WHERE username = '".addslashes($userid)."';";
	if(!mysql_query($query,$link))
	{
		$errstr = "MySQL Error: ".mysql_error($link);
		pc_db_close($link);
		exit($errstr);
	}
	
	//log一下
	$action = $currentuser["userid"]. " 驳回 " . $userid . " 的BLOG申请(www)";
	if($applyAgain == 3)
		$content = $action . "；\n并不允许再申请BLOG。";
	else
		$content = "";
	pc_logs($link , $action , $content , $userid );
}

if($_GET["userid"])
{
	if($_GET["act"] == "y")
		pc_add_users($link,$_GET["userid"],$_GET["pcname"],$_GET["manual"],$_GET["blogtype"],$_GET["groupmanager"]);
	elseif($_GET["act"] == "r")
		pc_reject_apply($link,$_GET["userid"],2);
	elseif($_GET["act"] == "d")
		pc_reject_apply($link,$_GET["userid"],3);
	else
	{
	}
}
$pno = $_GET["pno"];
$pno = intval($pno);
if($pno < 1) $pno = 1;
$type = $_GET["type"];
$type = intval($type);
if(!isset($_GET["type"]) || ($type != 0 && $type != 2 && $type != 3))
	$type = 1;

$pagesize = 20;
$start = ($pno - 1) * $pagesize;

if($_GET["act"] == "q")
	$newApps = pc_apply_users($link,$type,$start,$pagesize,$_GET["userid"],$_GET["pcname"]);
else
	$newApps = pc_apply_users($link,$type,$start,$pagesize);

pc_html_init("gb2312" , $pcconfig["BBSNAME"]."Blog新申请用户管理");
pc_admin_navigation_bar();
?>
<script language="javascript">
<!--
function bbsconfirm(url,infor){
	if(confirm(infor)){
		window.location.href=url;
		return true;
		}
	return false;
}
-->
</script>
<br />
<p align="center">BLOG新申请用户管理</p>
<center>
<table cellspacing=0 cellpadding=3 class=t1 width="95%">
<tr>
	<td width="30" class="t2">序号</td>
	<td class="t2" colspan="2">BLOG申请</td>
</tr>
<?php
	foreach($newApps as $newApp)
	{
		$start ++;
?>
<tr>
	<td class="t3" valign="middle" rowspan="5"><?php echo $start; ?></td>
	<td class="t3" width="25">ID</td>
	<td class="t5"><a href="/bbsqry.php?userid=<?php echo $newApp[username]; ?>"><?php echo $newApp[username]; ?></a></td>
</tr>
<tr>
	<td class="t3">名称</td>
	<td class="t5"><?php echo html_format($newApp[appname]); ?></td>
</tr><tr>
	<td class="t3">介绍</td>
	<td class="t5"><?php echo html_format($newApp[appself],TRUE); ?></td>
</tr>
<tr>
	<td class="t3">规划</td>
	<td class="t5"><?php echo html_format($newApp[appdirect],TRUE); ?></td>
</tr>
<tr>
	<td class="t3" colspan="2">
	FROM: <?php echo html_format($newApp[hostname]); ?> 
	@ <?php echo time_format($newApp[apptime]); ?>
	&nbsp;
<?php
	if($type == 1)
	{
?>	
	[<a href="#" onclick="bbsconfirm('<?php echo $_SERVER["PHP_SELF"]."?pno=".$pno."&type=".$type."&act=y&userid=".rawurlencode($newApp[username])."&pcname=".rawurlencode($newApp[appname]); ?>' , '通过<?php echo $newApp[username]; ?>的申请?')"><font color=red>通过</font></a>]
	[<a href="#" onclick="bbsconfirm('<?php echo $_SERVER["PHP_SELF"]."?pno=".$pno."&type=".$type."&act=r&userid=".rawurlencode($newApp[username]); ?>' , '驳回<?php echo $newApp[username]; ?>的申请?')"><font color=red>驳回</font></a>]
	[<a href="#" onclick="bbsconfirm('<?php echo $_SERVER["PHP_SELF"]."?pno=".$pno."&type=".$type."&act=d&userid=".rawurlencode($newApp[username]); ?>' , '驳回<?php echo $newApp[username]; ?>的申请(并不允许其再申请)?')"><font color=red>驳回并不允许再申请</font></a>]
<?php
	}
?>
	</td>
</tr>
<?php		
	}
?>
</table>
<?php
	if($type == 1)
	{
?>
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<b>[手动添加]</b>
<input type="hidden" name="act" value="y">
<input type="hidden" name="type" value="1">
<input type="hidden" name="manual" value="1">
用户名：
<select name="blogtype" class="f1">
<?php
    $types = array_keys ($pcconfig["TYPES"]);
    foreach ($types as $type) {
        if ($type=="normal")
            echo '<option value="normal" selected>(NORMAL)</option>';
        else
            echo '<option value="'.$type.'">'.$pcconfig["TYPES"][$type].'.</option>';      
    }
?>
</select>
<input type="text" class="f1" size="20" name="userid">
BLOG名：
<input type="text" class="f1" size="20" name="pcname">
(
群体Blog管理员：
<input type="text" class="f1" size="20" name="groupmanager">
)
<input type="submit" class="f1" value="添加">
</form>
<?php
	}
?>	
<form action="<?php echo $_SERVER["PHP_SELF"]; ?>" method="get">
<b>[用户查询]</b>
<input type="hidden" name="act" value="q">
<input type="hidden" name="type" value="<?php echo $type; ?>">
用户名：
<input type="text" class="f1" size="20" name="userid">
BLOG名：
<input type="text" class="f1" size="20" name="pcname">
<input type="submit" class="f1" value="查询">
</form>	
</center>
<br />
<p align="center">
<?php
	if($pno >1 ) echo "<a href=\"".$_SERVER["PHP_SELF"]."?pno=".($pno-1)."&type=".$type."\">上一页</a>\n";
	if(count($newApps) == $pagesize ) echo "<a href=\"".$_SERVER["PHP_SELF"]."?pno=".($pno+1)."&type=".$type."\">下一页</a>\n";
?>
</p>
<p align="center">
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>">新申请</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?type=0">已通过申请</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?type=2">驳回的申请</a>
<a href="<?php echo $_SERVER["PHP_SELF"]; ?>?type=3">不允许再申请用户</a>
</p>
<?php
pc_db_close($link);
pc_admin_navigation_bar();
html_normal_quit();
?>
