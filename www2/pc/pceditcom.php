<?php
	/*
	** some comments actions in personal corp.
	** @id:windinsn  Nov 19,2003
	*/
	require("pcfuncs.php");
	
	if ($loginok != 1)
		html_nologin();
	elseif(!strcmp($currentuser["userid"],"guest"))
	{
		html_init("gb2312");
		html_error_quit("guest 不能发表评论!");
		exit();
	}
	else
	{
		$act = $_GET["act"];
		$cid = (int)($_GET["cid"]);
		
		$link =	pc_db_connect();
		if($act == "del")
		{
			$query = "SELECT `username` , `uid` ,`nid` FROM comments WHERE `cid` = '".$cid."' LIMIT 0 , 1 ;";
			$result = mysql_query($query);
			$rows = mysql_fetch_array($result);
			mysql_free_result($result);
			if(strtolower($rows["username"])==strtolower($currentuser["userid"]) || pc_is_manager($currentuser))
			{
				$query = "DELETE FROM comments WHERE `cid` = '".$cid."' LIMIT 1;";
				mysql_query($query,$link);
				$query = "UPDATE nodes SET commentcount = commentcount - 1 WHERE `nid` = '".$rows["nid"]."' ;";
				mysql_query($query,$link);
			}
			else
			{
				$query = "SELECT `uid` FROM users WHERE `username` = '".$currentuser["userid"]."' AND `uid` = '".$rows["uid"]."' LIMIT 0 , 1;";
				$result = mysql_query($query,$link);
				if($rows1 = mysql_fetch_array($result))
				{
					$query = "DELETE FROM comments WHERE `cid` = '".$cid."' LIMIT 1;";
					mysql_query($query,$link);
					$query = "UPDATE nodes SET commentcount = commentcount - 1 WHERE `nid` = '".$rows["nid"]."' ;";
					mysql_query($query,$link);
				}
				@mysql_free_result($result);
			}
			pc_return("pccon.php?id=".$rows["uid"]."&nid=".$rows["nid"]."&s=all");
		}
		elseif($act == "edit")
		{
			$query = "SELECT `subject`,`body`,`htmltag`,`uid`,`nid` FROM comments WHERE `cid` = '".$cid."' AND `username` = '".$currentuser["userid"]."' LIMIT 0 , 1 ;";
			$result = mysql_query($query);
			$rows = mysql_fetch_array($result);
			mysql_free_result($result);
			if(!$rows)
			{
				pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
				html_error_quit("所选择的评论不存在!");
				exit();
			}
			if(!pc_can_comment($link , $rows["uid"]))
			{
				html_error_quit("对不起，您尚无该BLOG的评论权限！");
				exit();
			}	
		
			//判定评论文章是否用了编辑器，根据情况调入 windinsn feb 22 , 2004
			if($rows["htmltag"])
				pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog","","","",1);		
			else
				pc_html_init("gb2312",$pcconfig["BBSNAME"]."Blog");
?>
<br><center>
<form name="postform" action="pceditcom.php?act=edit2&cid=<?php echo $cid; ?>&id=<?php echo $rows["uid"]; ?>&nid=<?php echo $rows["nid"]; ?>" method="post" onsubmit="return submitwithcopy();">
<table cellspacing="0" cellpadding="5" width="90%" border="0" class="t1">
<tr>
	<td class="t2">修改评论</td>
</tr>
<tr>
	<td class="t8">
	主题
	<input class="f1" maxlength="200" type="text" name="subject" size="100" value="<?php echo htmlspecialchars(stripslashes($rows["subject"])); ?>">
	</td>
</tr>
<tr>
	<td class="t13">心情符号</td>
</tr>
<tr>
	<td class="t5"><?php @require("emote.html"); ?></td>
</tr>
<tr>
	<td class="t11">
	<input type="checkbox" name="htmltag" value=1 <?php if(strstr($rows["body"],$pcconfig["NOWRAPSTR"]) || $rows["htmltag"] ) echo "checked"; ?> >使用HTML标记
	</td>
</tr>
<tr>
	<td class="t8"><textarea name="blogbody" class="f1" style="width:100%" rows="20" id="blogbody" wrap="physical"><?php
		if($rows["htmltag"])
			echo $pcconfig["EDITORALERT"];
		echo htmlspecialchars(stripslashes($rows["body"]." ")); 
	?></textarea></td>
</tr>
<tr>
	<td class="t2">
	<input type="button" name="ins" value="插入HTML" class="b1" onclick="return insertHTML();" />
	<input type="button" name="hil" value="高亮" class="b1" onclick="return highlight();" />
	<input type="submit" name="postbutton" id="postbutton" value="修改评论" class="b1">
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
</p>	
<?php			
		}
		elseif($act == "edit2")
		{
			$emote = intval($_POST["emote"]);
			$uid = intval($_GET["id"]);
			$nid = intval($_GET["nid"]);
			$useHtmlTag = ($_POST["htmltag"]==1)?1:0;
			$query = "UPDATE `comments` SET `subject` = '".addslashes($_POST["subject"])."',`changed` = '".date("YmdHis")."',`body` = '".addslashes(html_editorstr_format($_POST["blogbody"]))."' , `emote` = '".$emote."' , `htmltag` = '".$useHtmlTag."' WHERE `cid` = '".$cid."' AND `username` = '".$currentuser["userid"]."' LIMIT 1 ;";
			mysql_query($query,$link);
			pc_return("pccon.php?id=".$uid."&nid=".$nid."&s=all");
		}
		
		pc_db_close($link);
		html_normal_quit();
	} 
?>
