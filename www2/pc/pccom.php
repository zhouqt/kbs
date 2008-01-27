<?php
	/*
	** some comments actions in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	require("pcfuncs.php");
	
	$nid = (int)($_GET["nid"]);
	$act = $_GET["act"];
	@$cid = (int)($_GET["cid"]);
		
	$link =	pc_db_connect();
	$query = "SELECT `access`,`uid` FROM nodes WHERE `nid` = '".$nid."' AND `type` != '1' AND `comment` != '0';";
	$result = mysql_query($query,$link);
	$rows = mysql_fetch_array($result);
	mysql_free_result($result);
		
	$pc = pc_load_infor($link,"",$rows["uid"]);
	if(!$pc)
        {
               	html_error_quit("对不起，您要查看的Blog不存在");
               	exit();
        }


	if(!$pc["ANONYCOMMENT"]) {
		if ($loginok != 1) {
			html_nologin();
			exit;
		}
		elseif(!strcmp($currentuser["userid"],"guest"))
		{
			html_init("gb2312");
			html_error_quit("guest 不能发表评论!\n<br>\n<a href=\"/\" target=\"_top\">现在登录</a>");
			exit();
		}
	}

	pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog","","","",1);		
		
	if(!$rows)
	{
		html_error_quit("所评论的文章不存在!");
		exit();
	}
	
	$uid = $rows["uid"];
	
	if(!$pc["ANONYCOMMENT"])
		if(!pc_can_comment($link , $uid))
		{
			html_error_quit("对不起，您尚无该BLOG的评论权限！");
			exit();
		}	
	
               
    $userPermission = pc_get_user_permission($currentuser,$pc);
	$sec = $userPermission["sec"];
	$pur = $userPermission["pur"];
	$tags = $userPermission["tags"];
	if(!$tags[$rows["access"]])
	{
		html_error_quit("对不起，您不能查看本条记录!");
		exit();
	}
		
		
	if($act == "pst")
	{
?>
<br><center>		
<form name="postform" action="pccom.php?act=add&nid=<?php echo $nid; ?>" method="post" onsubmit="return submitwithcopy();">
<table cellspacing="0" cellpadding="5" width="90%" border="0" class="t1">
<tr>
	<td class="t2">发表评论</td>
</tr>
<tr>
	<td class="t8">
	主题
	<input type="text" name="subject" maxlength="200" size="100" class="f1">
	</td>
</tr>
<tr>
	<td class="t13">心情符号</td>
</tr>
<tr>
	<td class="t5"><?php @require("emote.html"); ?></td>
</tr>
<tr>
	<td class="t11">内容
	<input type="checkbox" name="htmltag" value=1 checked>使用HTML标记
	</td>
</tr>
<tr>
	<td class="t8"><textarea name="blogbody" class="f1" style="width:100%" rows="20" id="blogbody" wrap="physical">
	<?php echo $pcconfig["EDITORALERT"].@$_POST["blogbody"]; ?>
	</textarea></td>
</tr>
<tr>
	<td class="t2">
	<input type="button" name="ins" value="插入HTML" class="b1" onclick="return insertHTML();" />
	<input type="button" name="hil" value="高亮" class="b1" onclick="return highlight();" />
	<input type="submit" name="postbutton" id="postbutton" value="发表评论" class="b1">
	<input type="button" value="返回上页" class="b1" onclick="doCancel();">
</tr>
</table>
</form></center>	
<p align="center">
<?php
    /**
     *    水木的web代码bbslib和cgi部分是修改于NJUWWWBBS-0.9，此部分
     * 代码遵循原有的nju www bbs的版权声明（GPL）。php部分的代码（
     * phplib以及php页面）不再遵循GPL，正在考虑使用其他开放源码的版
     * 权声明（BSD或者MPL之类）。
     *
     *   希望使用水木代码的Web站点加上powered by kbs的图标.该图标
     * 位于html/images/poweredby.gif目录,链接指向http://dev.kcn.cn
     * 使用水木代码的站点可以通过dev.kcn.cn获得代码的最新信息.
     *
     */
    powered_by_smth();
?>
<p>
<?php
	}
	else
	{
		if(!$_POST["subject"])
		{
			html_error_quit("请输入评论标题!");
			exit();
		}
		$ret = pc_add_comment($link,$pc,$nid,intval(($_POST["emote"])),$currentuser["userid"],$_POST["subject"],html_editorstr_format($_POST["blogbody"]),(($_POST["htmltag"]==1)?1:0),false);
        switch($ret) {
            case -6:
                html_error_quit("由于系统原因导致评论失败");
                break;
            case -9:
                echo "<script language=\"javascript\">alert('您的文章可能含有不当词汇，请等待管理员审核。');</script>";
                break;
            default:    
        }
            
?>
<script language="javascript">
window.location.href="pccon.php?id=<?php echo $uid; ?>&nid=<?php echo $nid; ?>";
</script>
<?php
    }		
	pc_db_close($link);
	html_normal_quit();
?>
